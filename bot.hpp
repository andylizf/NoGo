#include "gcem.hpp"
#include <chrono>
#include <iostream>
#include <random>
#include <vector>

#include "game.hpp"

using namespace std;
using namespace std::chrono;

// struct to represent a node in the Monte Carlo Tree
struct MCTSNode {
    State state;
    vector<Pos> available_actions;
    MCTSNode* parent;
    std::vector<MCTSNode*> children;
    int visit { 0 };
    double quality { 0 };

    constexpr MCTSNode(const State& state, MCTSNode* parent = nullptr)
        : state(state)
        , parent(parent)
    {
    }

    constexpr ~MCTSNode()
    {
        for (MCTSNode* child : children) {
            delete child;
        }
    }

    constexpr MCTSNode* add_child(const State& state)
    {
        MCTSNode* child = new MCTSNode(state, this);
        children.push_back(child);
        return child;
    }

    constexpr MCTSNode* best_child(double C)
    {
        double max_ucb1 = -2e50;
        MCTSNode* max_child = nullptr;
        for (MCTSNode* child : children) {
            double ucb1 = child->quality / child->visit
                + 2 * C * gcem::sqrt(gcem::log(2 * visit) / child->visit);
            if (ucb1 > max_ucb1) {
                max_ucb1 = ucb1;
                max_child = child;
            }
        }
        return max_child;
    }
};

// select the node to expand
inline constexpr MCTSNode* tree_policy(MCTSNode* node, double C)
{
    // if (!node->available_actions.size())
    //     node->available_actions = node->state.available_actions();

    while (!node->state.is_over() && node->children.size() == node->state.available_actions().size()) {
        node = node->best_child(C);
    }

    State state = node->state;
    if (!state.is_over()) {
        auto moves { state.available_actions() };
        auto move { moves[node->children.size()] };
        node = node->add_child(state.next_state(move));
    }
    return node;
}

inline std::mt19937 rng(std::random_device {}());
inline std::uniform_real_distribution<double> dist(0, 1);

// simulate the game from the expanded node
inline constexpr double default_policy(MCTSNode* node)
{
    State state = node->state;
    while (!state.is_over()) {
        auto moves = state.available_actions();
        int index = (int)moves.size() * dist(rng);
        state = state.next_state(moves[index]);
    }
    return state.is_over() == -node->state.role;
}

inline constexpr double default_policy2(MCTSNode* node)
{
    auto state { node->state };
    int n3 = state.available_actions().size();
    state.role.reverse();
    int n4 = state.available_actions().size();
    state.role.reverse();
    return n4 - n3;
}

// backpropagate the result of the simulation
inline constexpr void backup(MCTSNode* node, double reward)
{
    while (node) {
        node->visit++;
        node->quality += reward;
        node = node->parent;
        reward = -reward;
    }
}

inline Pos random_bot_player(const State& state)
{
    auto actions = state.available_actions();
    int index = (int)actions.size() * dist(rng);
    return actions[index];
}

inline constexpr auto mcts_bot_player_generator(double C)
{
    return [=](const State& state) {
        auto start = high_resolution_clock::now();
        MCTSNode* root = new MCTSNode(state);
        while (high_resolution_clock::now() - start < 990ms) {
            MCTSNode* expand_node = tree_policy(root, C);
            double reward = default_policy2(expand_node);
            backup(expand_node, reward);
        }
        return root->best_child(0)->state.moves.back();
    };
}
inline constexpr auto mcts_bot_player(State state)
{
    MCTSNode* root = new MCTSNode(state);
    for (int i = 0; i < 10; i++) {
        MCTSNode* expand_node = tree_policy(root, 0.1);
        double reward = default_policy2(expand_node);
        backup(expand_node, reward);
    }
    Pos best_move { root->best_child(0)->state.moves.back() };
    delete root;
    return best_move;
}

inline auto mcts_bot_player_a(State state)
{
    MCTSNode* root = new MCTSNode(state);
    for (int i = 0; i < 10000; i++) {
        MCTSNode* expand_node = tree_policy(root, 0.1);
        double reward = default_policy2(expand_node);
        backup(expand_node, reward);
    }
    Pos best_move { root->best_child(0)->state.moves.back() };
    delete root;
    return best_move;
}

#define SELECT
