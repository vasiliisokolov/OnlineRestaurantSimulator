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
    

public:
    
    void setDish(int in)
    {
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
            kitchen_access.unlock();
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            sentOrderToKitchen(ordersQueue);
        }
    }

    Order()
    {
        
        setDish(1 + rand() % 5);
        setTime(4 + rand() % 12);
    }

};

class Kitchen
{
public:

    std::vector<Order*> ordersQueue;
    Order* order;
    std::vector<Order*> distribution;

    void getOrder()
    {
        if (kitchen_access.try_lock())
        {
            if (ordersQueue[0] == nullptr)
            {
                std::this_thread::sleep_for(std::chrono::seconds(10));
                getOrder();
            }
            order = ordersQueue[0];
            removeOrder();
            kitchen_access.unlock();
            cooking();
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            getOrder();
        }
    }

    void cooking()
    {
        std::this_thread::sleep_for(std::chrono::seconds(order->getTime()));
        distribution_access.lock();
        distribution.push_back(order);
        distribution_access.unlock();
    }

    void removeOrder()
    {
        for (int i = 0; i < ordersQueue.size() - 2; i++)
        {
            ordersQueue[i] = ordersQueue[i + 1];
        }
        ordersQueue.pop_back();
    }

    Kitchen()
    {
        order = nullptr;
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
        if (distribution_access.try_lock())
        {
            for (int i = 0; i < kitchen->distribution.size(); i++)
            {
                delivery.push_back(kitchen->distribution[i]);
            }
            kitchen->distribution.clear();
            distribution_access.unlock();
            deliveryDone();
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::seconds(30));
            getDelivery();
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
    deliweryWorking.join();

    std::cout << "Restaurant Simulation!\n";
    for (int i = 0; i < 10; i++)
    {
        std::this_thread::sleep_for(std::chrono::seconds(5 + rand()%5));
        Order* order = new Order();
        std::cout << "New order: " << order->getDish() << " in " << order->getTime() << "seconds." << std::endl;
        order->sentOrderToKitchen(kitchen->ordersQueue);
        
    }

    delete courier;
    delete kitchen;
}