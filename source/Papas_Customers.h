#pragma once
#include <string>
#include "Papas_Utils.h"
#include <unordered_map>
namespace Papas{

    struct CustomerData
    {
        std::string name;
        std::vector<ItemOrder> items;
        int time;
        int CutPizzaIn;
    };

    class Customer{
        public:
        //Example functions for later's animations
        void SpawnCustomer();
        void AnimateCustomer();
    };

    std::unordered_map<int, CustomerData> map_customers = {
        {0,                               // Index
         {"Cooper",                       // Name
          {{{0, 0, 1, 1}, Pepperoni, 4}}, // Vector/array of items the customer wants (Coverage, Topping, Ammount)
          1,                              // Time (notch timer)
          4}},                            // Slices
        {1, {"Wally", {{{1, 1, 1, 1}, Anochovie, 8}}, 2, 8}},
        {2, {"Rita", {{{0, 1, 1, 0}, Mushroom, 6}}, 4, 4}},
        {3, {"Marty", {{{1, 1, 0, 0}, Olive, 6}}, 3, 4}},
        {4, {"Kingsley", {{{0, 0, 1, 1}, Pepperoni, 8}}, 4, 4}},
        {5, {"Timm", {{{0, 0, 1, 1}, Pepper, 4}}, 4, 6}},
        {6, {"Big Pauly", {{{1, 1, 0, 0}, Meat, 4}, {{0, 0, 1, 1}, Onion, 4}}, 3, 8}},
        {7, {"Penny", {{{1, 1, 1, 1}, Meat, 8}, {{1, 0, 0, 0}, Mushroom, 2}}, 2, 6}},
        {8, {"Maggie", {{{0, 0, 1, 1}, Pepper, 4}, {{1, 1, 0, 0}, Olive, 6}}, 2, 4}},
        {9, {"Taylor", {{{1, 1, 0, 0}, Pepper, 2}, {{1, 1, 0, 0}, Onion, 6}}, 3, 4}},
        {10, {"Sue", {{{1, 0, 0, 1}, Pepperoni, 6}, {{0, 1, 1, 0}, Mushroom, 6}}, 3, 6}}

    };
}