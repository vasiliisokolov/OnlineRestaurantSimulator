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
            std::cout << getDish() << " sent to Kitchen!" << std::endl;
            kitchen_access.unlock();
        }
        else
        {
            std::cout << getDish() << " Not access to Kitchen!" << std::endl;
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
            
            if (ordersQueue.size() == 0 || ordersQueue[0] == nullptr)
            {
                std::cout << "No orders." << std::endl;
                kitchen_access.unlock();
                std::this_thread::sleep_for(std::chrono::seconds(10));
                getOrder();
            }
            else
            {
                order = ordersQueue[0];
                std::cout << "Order: " << order->getDish() << " is moved to kitchen." << std::endl;
                removeOrder();
                kitchen_access.unlock();
                cooking();
                
            }
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
        std::cout << "Order: " << order->getDish() << " is ready." << std::endl;
        sendOrderToDelivery();
    }

    void sendOrderToDelivery()
    {
        if (distribution_access.try_lock())
        {
            distribution.push_back(order);
            distribution_access.unlock();
            std::cout << "Order: " << order->getDish() << " is ready." << std::endl;
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
    }
};

class Courier
{
public:
    //Kitchen* kitchen;
    int deliveryCount;
    
    std::vector<Order*> delivery;

    void getDelivery(std::vector<Order*> distribution)
    {
        std::this_thread::sleep_for(std::chrono::seconds(30));
        if (distribution_access.try_lock())
        {
            if (distribution.size() > 0)
            {
                for (int i = 0; i < distribution.size(); i++)
                {
                    delivery.push_back(distribution[i]);
                    std::cout << delivery[i]->getDish() << " handed over for delivery!" << std::endl;
                }
                distribution.clear();
                distribution_access.unlock();
                deliveryDone();
            }
            else
            {
                distribution_access.unlock();
                
                getDelivery(distribution);
            }
            
        }
        else
        {
            
            getDelivery(distribution);
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

    Courier()
    {
        //kitchen = inKitchen;
        deliveryCount = 0;
    }
};

int main()
{
    Kitchen* kitchen = new Kitchen();
    std::thread restaurantOpen(&Kitchen::getOrder, kitchen);
    restaurantOpen.detach();

    Courier* courier = new Courier();
    std::thread deliweryWorking (&Courier::getDelivery, courier, kitchen->distribution);
    deliweryWorking.join();

    std::cout << "Restaurant Simulation!\n";
    for (int i = 0; i < 10; i++)
    {
        std::this_thread::sleep_for(std::chrono::seconds(5 + rand()%5));
        Order* order = new Order();
        std::cout << "New order: " << order->getDish() << " in " << order->getTime() << " seconds." << std::endl;
        order->sentOrderToKitchen(kitchen->ordersQueue);
        
    }

    delete courier;
    delete kitchen;
}