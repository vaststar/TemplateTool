#include <catch2/catch_test_macros.hpp>

#include <string>
#include <utility>

#include <ucf/utilities/NotificationHelper/NotificationHelper.h>

class IObaserverCallback{
public: 
virtual void onReceiveData(std::string data) = 0;
virtual ~IObaserverCallback() = default;
};

class Observer: public ucf::utilities::NotificationHelper<IObaserverCallback>
{
public:
void fireMyData(std::string&& data){
    fireNotification(&IObaserverCallback::onReceiveData, std::move(data));
}
};

class Listener: public IObaserverCallback
{
public: 
    virtual void onReceiveData(std::string data) override
    {
        receiveData = std::move(data);
    }
    std::string receiveData;
};

TEST_CASE( "test notification", "[NotificationHelperTests]" ) {

    std::shared_ptr<Observer> A = std::make_shared<Observer>();
    std::shared_ptr<Listener> B = std::make_shared<Listener>();
    std::shared_ptr<Listener> C = std::make_shared<Listener>();
    A->registerCallback(B);
    A->registerCallback(C);

    std::string data(128, 'x');
    const auto expectedData = data;
    A->fireMyData(std::move(data));

    REQUIRE(expectedData == B->receiveData);
    REQUIRE(expectedData == C->receiveData);

}
