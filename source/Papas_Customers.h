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
        {1,                               // Index
         {"Cooper",                       // Name
          {{{0, 0, 1, 1}, Pepperoni, 4}}, // Vector/array of items the customer wants (Coverage, Topping, Ammount)
          1,                              // Time (notch timer)
          4}},                            // Slices
        {2, {"Wally", {{{1, 1, 1, 1}, Anochovie, 8}}, 2, 8}},
        {3, {"Rita", {{{0, 1, 1, 0}, Mushroom, 6}}, 4, 4}},
        {4, {"Marty", {{{1, 1, 0, 0}, Olive, 6}}, 3, 4}},
        {5, {"Kingsley", {{{0, 0, 1, 1}, Pepperoni, 8}}, 4, 4}},
        {6, {"Timm", {{{0, 0, 1, 1}, Pepper, 4}}, 4, 6}},
        {7, {"Big Pauly", {{{1, 1, 0, 0}, Meat, 4}, {{0, 0, 1, 1}, Onion, 4}}, 3, 8}},
        {8, {"Penny", {{{1, 1, 1, 1}, Meat, 8}, {{1, 0, 0, 0}, Mushroom, 2}}, 2, 6}},
        {9, {"Maggie", {{{0, 0, 1, 1}, Pepper, 4}, {{1, 1, 0, 0}, Olive, 6}}, 2, 4}},
        {10, {"Taylor", {{{1, 1, 0, 0}, Pepper, 2}, {{1, 1, 0, 0}, Onion, 6}}, 3, 4}},
        {11, {"Sue", {{{1, 0, 0, 1}, Pepperoni, 6}, {{0, 1, 1, 0}, Mushroom, 6}}, 3, 6}},
        {12, {"Allan", {{{1, 0, 0, 1}, Pepperoni, 4}, {{0, 1, 1, 0}, Meat, 4}}, 4, 6}},
        {13, {"Mindy", {{{1, 0, 0, 0}, Mushroom, 4}, {{1, 1, 0, 0}, Anochovie, 6}}, 5, 8}},
        {14, {"Chuck", {{{1, 1, 1, 1}, Pepperoni, 8}, {{0, 1, 1, 0}, Meat, 4}}, 2, 6}},
        {15, {"Greg", {{{0, 1, 1, 1}, Pepperoni, 6}, {{0, 0, 1, 0}, Mushroom, 4}}, 4, 4}},
        {16, {"Robby", {{{0, 1, 1, 1}, Mushroom, 6}, {{0, 0, 1, 1}, Pepper, 6}}, 4, 6}},
        {17, {"Mary", {{{1, 1, 1, 1}, Pepperoni, 8}}, 2, 4}},
        {18, {"Mitch", {{{1, 1, 0, 0}, Pepperoni, 4}, {{1, 0, 0, 0}, Olive, 2}, {{1, 1, 0, 0}, Anochovie, 4}}, 2, 4}},
        {19, {"Prudence", {{{1, 0, 0, 0}, Mushroom, 5}, {{0, 1, 0, 0}, Onion, 3}}, 2, 6}}
        // {20, {"Sue", {{{1, 0, 0, 1}, Pepperoni, 6}, {{0, 1, 1, 0}, Mushroom, 6}}, 3, 6}},
        // {11, {"Sue", {{{1, 0, 0, 1}, Pepperoni, 6}, {{0, 1, 1, 0}, Mushroom, 6}}, 3, 6}},
        // {11, {"Sue", {{{1, 0, 0, 1}, Pepperoni, 6}, {{0, 1, 1, 0}, Mushroom, 6}}, 3, 6}},
        // {11, {"Sue", {{{1, 0, 0, 1}, Pepperoni, 6}, {{0, 1, 1, 0}, Mushroom, 6}}, 3, 6}},
        // {11, {"Sue", {{{1, 0, 0, 1}, Pepperoni, 6}, {{0, 1, 1, 0}, Mushroom, 6}}, 3, 6}},
        // {11, {"Sue", {{{1, 0, 0, 1}, Pepperoni, 6}, {{0, 1, 1, 0}, Mushroom, 6}}, 3, 6}},
        // {11, {"Sue", {{{1, 0, 0, 1}, Pepperoni, 6}, {{0, 1, 1, 0}, Mushroom, 6}}, 3, 6}},
        // {11, {"Sue", {{{1, 0, 0, 1}, Pepperoni, 6}, {{0, 1, 1, 0}, Mushroom, 6}}, 3, 6}},

    };
}