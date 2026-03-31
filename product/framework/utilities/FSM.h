#pragma once

#include <memory>
#include <queue>
#include <mutex>
#include <concepts>
#include <string>

struct Event {
    virtual ~Event() = default;
};

// -----------------------------------------------------------------------------
// Concept 强约束
// -----------------------------------------------------------------------------
template <typename Ctx>
concept FsmContext = std::is_class_v<Ctx>;

template <typename S, typename Ctx>
concept FsmState =
    FsmContext<Ctx> &&
    requires(S s, Ctx& ctx, const Event& e) {
        { s.on_enter(ctx) }   -> std::same_as<void>;
        { s.on_update(ctx) }  -> std::same_as<void>;
        { s.on_event(ctx, e) }-> std::same_as<void>;
        { s.on_exit(ctx) }    -> std::same_as<void>;
        { s.name() }          -> std::convertible_to<std::string>;
    };

// -----------------------------------------------------------------------------
// FSM 前向声明
// -----------------------------------------------------------------------------
template <FsmContext Ctx>
class FSM;

// -----------------------------------------------------------------------------
// State 基类
// -----------------------------------------------------------------------------
template <FsmContext Context>
class StateBase {
    friend class FSM<Context>;
public:
    virtual ~StateBase() = default;

    virtual void on_enter(Context&)   = 0;
    virtual void on_update(Context&)  = 0;
    virtual void on_event(Context&, const Event&) = 0;
    virtual void on_exit(Context&)    = 0;
    virtual std::string name() const  = 0;

protected:
    template <FsmState<Context> NextState, typename... Args>
    void switch_to(Args&&... args);

    void send_event(std::unique_ptr<Event> evt);
    std::weak_ptr<Context> get_context_weak() const;

private:
    FSM<Context>* fsm_ = nullptr;
};

// -----------------------------------------------------------------------------
// FSM 最终版
// -----------------------------------------------------------------------------
template <FsmContext Context>
class FSM {
    friend class StateBase<Context>;
public:
    explicit FSM(std::weak_ptr<Context> ctx)
        : ctx_weak_(std::move(ctx)) {}

    // 启动状态机
    template <FsmState<Context> InitialState, typename... Args>
    void start(Args&&... args);

    // -------------------------------------------------------------------------
    // 【重要】
    // update 必须由【单线程】在【主循环】中调用！
    // 不支持多线程并发 update
    // -------------------------------------------------------------------------
    void update();

    // 线程安全：任意线程可发送事件
    void send_event(std::unique_ptr<Event> evt);
    std::string current_state_name() const;

    // 禁止拷贝
    FSM(const FSM&) = delete;
    FSM& operator=(const FSM&) = delete;
    FSM(FSM&&) = default;
    FSM& operator=(FSM&&) = default;

private:
    std::shared_ptr<Context> lock_ctx() const {
        return ctx_weak_.lock();
    }

    std::weak_ptr<Context> get_context_weak() const {
        return ctx_weak_;
    }

    // 状态切换（update 期间禁止调用，防止嵌套）
    void change_state(std::unique_ptr<StateBase<Context>> next);

private:
    std::weak_ptr<Context>             ctx_weak_;
    std::unique_ptr<StateBase<Context>> current_state_;
    std::queue<std::unique_ptr<Event>> event_queue_;
    mutable std::mutex                 event_mutex_;

    // 防嵌套切换：仅在单线程 update 中使用，无需锁
    bool is_updating_ = false;
};

// -----------------------------------------------------------------------------
// 模板实现
// -----------------------------------------------------------------------------

template <FsmContext Ctx>
template <FsmState<Ctx> NextState, typename... Args>
void StateBase<Ctx>::switch_to(Args&&... args) {
    fsm_->change_state(
        std::make_unique<NextState>(std::forward<Args>(args)...)
    );
}

template <FsmContext Ctx>
void StateBase<Ctx>::send_event(std::unique_ptr<Event> evt) {
    fsm_->send_event(std::move(evt));
}

template <FsmContext Ctx>
std::weak_ptr<Ctx> StateBase<Ctx>::get_context_weak() const {
    return fsm_->get_context_weak();
}

template <FsmContext Ctx>
template <FsmState<Ctx> InitialState, typename... Args>
void FSM<Ctx>::start(Args&&... args) {
    auto st = std::make_unique<InitialState>(std::forward<Args>(args)...);
    st->fsm_ = this;
    change_state(std::move(st));
}

template <FsmContext Ctx>
void FSM<Ctx>::change_state(std::unique_ptr<StateBase<Ctx>> next) {
    // 禁止在 update 内部嵌套切换状态（防死循环/崩溃）
    if (is_updating_) return;

    auto ctx = lock_ctx();
    if (!ctx || !next) return;

    if (current_state_)
        current_state_->on_exit(*ctx);

    current_state_ = std::move(next);
    current_state_->on_enter(*ctx);
}

template <FsmContext Ctx>
void FSM<Ctx>::update() {
    auto ctx = lock_ctx();
    if (!ctx || !current_state_) return;

    // 标记：禁止 update 内部切换状态
    is_updating_ = true;

    // 线程安全地取出所有事件
    std::queue<std::unique_ptr<Event>> local_queue;
    {
        std::lock_guard lock(event_mutex_);
        local_queue.swap(event_queue_);
    }

    // 按顺序分发
    while (!local_queue.empty()) {
        auto e = std::move(local_queue.front());
        local_queue.pop();
        current_state_->on_event(*ctx, *e);
    }

    // 执行当前状态逻辑
    current_state_->on_update(*ctx);

    // 解除标记
    is_updating_ = false;
}

template <FsmContext Ctx>
void FSM<Ctx>::send_event(std::unique_ptr<Event> evt) {
    if (!evt) return;
    std::lock_guard lock(event_mutex_);
    event_queue_.push(std::move(evt));
}

template <FsmContext Ctx>
std::string FSM<Ctx>::current_state_name() const {
    return current_state_ ? current_state_->name() : "none";
}
