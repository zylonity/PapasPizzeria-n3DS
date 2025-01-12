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
        {19, {"Prudence", {{{1, 0, 0, 0}, Mushroom, 5}, {{0, 1, 0, 0}, Onion, 3}}, 2, 6}},
        {20, {"James", {{{1, 1, 0, 0}, Meat, 4}, {{0, 1, 1, 0}, Olive, 8}}, 2, 4}},
        {21, {"Cecilia", {{{1, 1, 1, 6}, Mushroom, 4}, {{0, 1, 1, 1}, Pepper, 3}, {{1, 1, 0, 1}, Onion, 3}}, 2, 8}},
        {22, {"Mandi", {{{1, 1, 0, 0}, Pepperoni, 4}, {{1, 0, 1, 1}, Mushroom, 6}}, 4, 8}},
        {23, {"Sasha", {{{0, 1, 0, 0}, Pepper, 4}, {{1, 1, 1, 1}, Olive, 8}}, 4, 8}},
        {24, {"Olga", {{{1, 1, 1, 0}, Meat, 6}, {{0, 0, 1, 0}, Mushroom, 4}, {{0, 0, 1, 0}, Pepper, 2}}, 6, 4}},
        {25, {"Franco", {{{1, 1, 1, 1}, Pepperoni, 8}, {{1, 0, 1, 1}, Olive, 3}}, 4, 8}},
        {26, {"Tohru", {{{0, 0, 1, 1}, Mushroom, 6}, {{1, 1, 1, 1}, Anochovie, 8}}, 2, 8}},
        {27, {"Clair", {{{1, 1, 1, 1}, Pepperoni, 4}, {{0, 0, 1, 1}, Mushroom, 6}, {{0, 1, 0, 0}, Pepper, 4}}, 4, 4}},
        {28, {"Clover", {{{1, 1, 1, 1}, Pepperoni, 8}}, 4, 8}},
        {29, {"Hugo", {{{1, 1, 0, 0}, Meat, 4}, {{0, 1, 0, 0}, Pepper, 4}}, 4, 6}},
        {30, {"Peggy", {{{1, 1, 1, 1}, Onion, 4}, {{1, 1, 1, 0}, Olive, 6}}, 3, 8}},
        {31, {"Carlo Romano", {{{1, 1, 0, 0}, Meat, 4}, {{0, 0, 1, 1}, Mushroom, 6}, {{0, 1, 0, 0}, Pepper, 4}}, 4, 8}},
        {32, {"Little Edoardo", {{{1, 1, 1, 1}, Onion, 4}, {{1, 1, 1, 1}, Olive, 4}, {{1, 1, 1, 1}, Anochovie, 4}}, 5, 8}},
        {33, {"Gino Romano", {{{1, 1, 1, 1}, Pepperoni, 8}, {{1, 1, 1, 1}, Onion, 4}, {{1, 1, 1, 1}, Olive, 4}}, 4, 8}},
        {34, {"Bruna Romano", {{{1, 0, 0, 0}, Pepperoni, 2}, {{1, 1, 1, 1}, Meat, 4}, {{1, 1, 1, 1}, Olive, 4}}, 5, 4}},
        {35, {"SargeFan!", {{{1, 1, 1, 1}, Onion, 12}}, 5, 6}},
        // Fat fuck
        {36, {"PAPA LOUIE!", {{{1, 0, 0, 0}, Pepperoni, 2},
                              {{0, 1, 0, 0}, Meat, 2},
                              {{0, 0, 1, 0}, Mushroom, 2},
                              {{0, 0, 0, 1}, Pepper, 2},
                              {{1, 0, 0, 0}, Onion, 2},
                              {{0, 1, 0, 0}, Olive, 2},
                              {{0, 0, 1, 0}, Anochovie, 2}}, 4, 4}}
        

    };
}