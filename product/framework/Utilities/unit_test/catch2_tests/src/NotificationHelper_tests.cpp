#include <catch2/catch_test_macros.hpp>
#include "Utilities/NotificationHelper/NotificationHelper.h"

class IObaserverCallback{
public: 
virtual void onReceiveData(int&& a) = 0; 
virtual ~IObaserverCallback() = default;
};

class Observer: public NotificationHelper<IObaserverCallback>
{
public:
void fireMyData(int&& data){
    fireNotification(&IObaserverCallback::onReceiveData, std::move(data));
}
};

class Listener: public IObaserverCallback
{
public: 
    virtual void onReceiveData(int&& a) override
    {
        receiveData = a;
    }
    int receiveData{0};
};

TEST_CASE( "test notification", "[NotificationHelperTests]" ) {

    std::shared_ptr<Observer> A = std::make_shared<Observer>();
    std::shared_ptr<Listener> B = std::make_shared<Listener>();
    std::shared_ptr<Listener> C = std::make_shared<Listener>();
    A->registerCallback(B);
    A->registerCallback(C);
    int data = 8;
    A->fireMyData(std::move(data));
    REQUIRE(8 == B->receiveData);
    REQUIRE(8 == C->receiveData);

}