#include <cmath>
#include <iostream>
#include <random>
#include <vector>

#include "game.hpp"

// struct to represent a node in the Monte Carlo Tree
struct MCTSNode {
    State state;
    vector<Pos> available_actions;
    MCTSNode* parent;
    std::vector<MCTSNode*> children;
    int visit { 0 };
    double quality { 0 };

    MCTSNode(const State& state, MCTSNode* parent = nullptr)
        : state(state)
        , parent(parent)
    {
    }

    ~MCTSNode()
    {
        for (MCTSNode* child : children) {
            delete child;
        }
    }

    MCTSNode* addChild(const State& state)
    {
        MCTSNode* child = new MCTSNode(state, this);
        children.push_back(child);
        return child;
    }

    MCTSNode* best_child(double C)
    {
        double max_ucb1 = -2e50;
        MCTSNode* max_child = nullptr;
        for (MCTSNode* child : children) {
            double ucb1 = child->quality / child->visit
                + 2 * C * sqrt(log(2 * visit) / child->visit);
            if (ucb1 > max_ucb1) {
                max_ucb1 = ucb1;
                max_child = child;
            }
        }
        return max_child;
    }
};

vector<int> vec;

// select the node to expand
MCTSNode* tree_policy(MCTSNode* node)
{
    vec.push_back(5);
    //if (!node->available_actions.size())
    //    node->available_actions = node->state.available_actions();

    vec.push_back(6);
    while (!node->state.is_over() && node->children.size() == node->state.available_actions().size()) {
        vec.push_back(7);
        node = node->best_child(0.1);
    }

    if (!node->state.is_over()) {
        auto moves = node->state.available_actions();
        auto move = moves[node->children.size()];
        State state = node->state;
        state.put(move);
        node = node->addChild(state);
    }
    return node;
}

std::mt19937 rng(std::random_device {}());
std::uniform_real_distribution<double> dist(0, 1);

// simulate the game from the expanded node
double default_policy(MCTSNode* node)
{
    State state = node->state;
    bool isblack = state.isblack;
    while (!state.is_over()) {
        auto moves = state.available_actions();
        int index = dist(rng) * moves.size();
        state.put(moves[index]);
    }
    return isblack == state.isblack;
}

// backpropagate the result of the simulation
void backup(MCTSNode* node, double reward)
{
    while (node) {
        node->visit++;
        node->quality += reward;
        node = node->parent;
        reward = -reward;
    }
}

// function to perform the Monte Carlo Tree Search
MCTSNode* MCTS(MCTSNode* root, int maxIterations)
{
    for (int i = 0; i < maxIterations; i++) {
        MCTSNode* expand_node = tree_policy(root);
        double reward = default_policy(expand_node);
        backup(expand_node, reward);
    }
    return root->best_child(0);
}

inline Pos mcts_bot_player(const State& state)
{
    MCTSNode* root = new MCTSNode(state);
    MCTSNode* bestNode = MCTS(root, 1000);
    return bestNode->state.moves.back();
};