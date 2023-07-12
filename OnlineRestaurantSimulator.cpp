#include <iostream>
#include <thread>
#include <vector>
#include <mutex>

std::mutex distribution_access;
std::mutex kitchen_access;



enum dishType
{
    none,
    pizza,
    soup,
    steak,
    salad,
    sushi
};

class Order
{
    dishType dish;
    int cookingTime;
    int orderNumber;
    
public:
    
    void setNumber(int inNum)
    {
        orderNumber = inNum;
    }

    int getNumber()
    {
        return orderNumber;
    }

    void setDish(int in)
    {
        if (in <= 0) in = 1;
        if (in > 5) in = 5;
        switch (in)
        {
        case 1:
            dish = pizza;
            break;
        case 2:
            dish = soup;
            break;
        case 3:
            dish = steak;
            break;
        case 4:
            dish = salad;
            break;
        case 5:
            dish = sushi;
            break;

        }
    }

    void setTime(int inTime)
    {
        cookingTime = inTime;
    }

    int getTime()
    {
        return cookingTime;
    }
    
    std::string getDish()
    {
        switch (dish)
        {
        case none:
            return "none";
            break;
        case pizza:
            return "pizza";
            break;
        case soup:
            return "soup";
            break;
        case steak:
            return "steak";
            break;
        case salad:
            return "salad";
            break;
        case sushi:
            return "sushi";


        }
    }

    void sentOrderToKitchen(std::vector<Order*>& ordersQueue)
    {
        if (kitchen_access.try_lock())
        {
            ordersQueue.push_back(this);
            std::cout << "Order #" << getNumber() << ": " << getDish() << " sent to Kitchen!" << std::endl;
            kitchen_access.unlock();
        }
        else
        {
            std::cout << getDish() << " Not access to Kitchen!" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(5));

            sentOrderToKitchen(ordersQueue);
        }
    }

    Order(int inNumber)
    {
        setNumber(inNumber);
        setDish(1 + rand() % 5);
        setTime(4 + rand() % 12);
    }

};

class Kitchen
{
public:
    int dishCount;

    std::vector<Order*> ordersQueue;
    Order* order;
    std::vector<Order*> distribution;

    void getOrder()
    {
        while (dishCount < 10)
        {
            if (kitchen_access.try_lock())
            {

                if (ordersQueue.size() == 0 || ordersQueue[0] == nullptr)
                {
                    std::cout << "No orders." << std::endl;
                    kitchen_access.unlock();
                    std::this_thread::sleep_for(std::chrono::seconds(5));
                    
                }
                else
                {
                    order = ordersQueue[0];
                    std::cout << "Order #" << order->getNumber() << ": " << order->getDish() << " is moved to kitchen." << std::endl;
                    removeOrder();
                    kitchen_access.unlock();
                    cooking();
                    dishCount++;
                    
                }
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::seconds(5));
                
            }
            
        }
    }

    void cooking()
    {
        
        std::this_thread::sleep_for(std::chrono::seconds(order->getTime()));
        std::cout << "Cooking order# " << order->getNumber() << ": " << order->getDish() << "!" << std::endl;
        sendOrderToDelivery();
    }

    void sendOrderToDelivery()
    {
        if (distribution_access.try_lock())
        {
            distribution.push_back(order);
            
            std::cout << "Order #" << distribution.back()->getNumber() << ": " << distribution.back()->getDish() << " is ready." << std::endl;
            distribution_access.unlock();
        }
        else
        {
            sendOrderToDelivery();
        }
    }
    void removeOrder()
    {
        for (int i = 1; i < ordersQueue.size(); i++)
        {
            ordersQueue[i - 1] = ordersQueue[i];
        }
        ordersQueue.pop_back();
        
    }

    Kitchen()
    {
        order = nullptr;
        dishCount = 0;
    }
};

class Courier
{
public:
    Kitchen* kitchen;
    int deliveryCount;
    
    std::vector<Order*> delivery;

    void getDelivery()
    {
        while (deliveryCount < 10)
        {
            
            if (distribution_access.try_lock())
            {
                if (kitchen->distribution.size() > 0)
                {
                    for (int i = 0; i < kitchen->distribution.size(); i++)
                    {
                        delivery.push_back(kitchen->distribution[i]);
                        std::cout << delivery[i]->getDish() << " handed over for delivery!" << std::endl;
                    }
                    kitchen->distribution.clear();
                    distribution_access.unlock();
                    deliveryDone();
                    std::this_thread::sleep_for(std::chrono::seconds(30));
                }
                else
                {
                    std::cout << "No orders to delivery!" << std::endl;
                    distribution_access.unlock();
                    std::this_thread::sleep_for(std::chrono::seconds(5));
                    
                }

            }
            else
            {
                std::this_thread::sleep_for(std::chrono::seconds(30));
            }
            
        }
    }

    void deliveryDone()
    {
        
        for (int i = 0; i < delivery.size(); i++)
        {
            deliveryCount++;
            std::cout << "Order #" << deliveryCount << ". " << delivery[i]->getDish() << " delivered!" << std::endl;
            
            delete delivery[i];
        }
        delivery.clear();
    }

    Courier(Kitchen* inKitchen)
    {
        kitchen = inKitchen;
        deliveryCount = 0;
    }
};

int main()
{
    Kitchen* kitchen = new Kitchen();
    std::thread restaurantOpen(&Kitchen::getOrder, kitchen);
    restaurantOpen.detach();

    Courier* courier = new Courier(kitchen);
    std::thread deliweryWorking (&Courier::getDelivery, courier);
    deliweryWorking.detach();

    std::cout << "Restaurant Simulation!\n";
    for (int i = 0; i < 10 && courier->deliveryCount < 10; i++)
    {
        std::this_thread::sleep_for(std::chrono::seconds(5 + rand()%5));
        Order* order = new Order(i+1);
        std::cout << "New order: " << order->getDish() << " in " << order->getTime() << " seconds." << std::endl;
        order->sentOrderToKitchen(kitchen->ordersQueue);
        
    }

    delete courier;
    delete kitchen;
}