/**
 * Copyright © 2020 IBM Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include "types.hpp"

#include <fmt/format.h>

#include <nlohmann/json.hpp>
#include <phosphor-logging/log.hpp>

#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <numeric>

namespace phosphor::fan::control::json
{

using json = nlohmann::json;
using namespace phosphor::logging;

/**
 * @brief Function used in creating action objects
 *
 * @param[in] jsonObj - JSON object for the action
 *
 * Creates an action object given the JSON configuration
 */
template <typename T>
std::unique_ptr<T> createAction(const json& jsonObj)
{
    return std::make_unique<T>(jsonObj);
}

/**
 * @class ActionBase - Base action object
 *
 * Base class for fan control's event actions
 */
class ActionBase
{
  public:
    ActionBase() = delete;
    ActionBase(const ActionBase&) = delete;
    ActionBase(ActionBase&&) = delete;
    ActionBase& operator=(const ActionBase&) = delete;
    ActionBase& operator=(ActionBase&&) = delete;
    virtual ~ActionBase() = default;
    explicit ActionBase(const std::string& name) : _name(name)
    {}

    /**
     * @brief Get the action function to perform
     *
     * An action function is a function associated to the derived action object
     * that performs a specific task against fan control that's configured by
     * a user.
     *
     * @return Action function
     */
    virtual const Action getAction() = 0;

    /**
     * @brief Get the action's name
     *
     * @return Name of the action
     */
    inline const auto& getName() const
    {
        return _name;
    }

  private:
    /* The action's name that is used within the JSON configuration */
    const std::string _name;
};

/**
 * @class ActionFactory - Factory for actions
 *
 * Factory that registers and retrieves actions based on a given name.
 */
class ActionFactory
{
  public:
    ActionFactory() = delete;
    ActionFactory(const ActionFactory&) = delete;
    ActionFactory(ActionFactory&&) = delete;
    ActionFactory& operator=(const ActionFactory&) = delete;
    ActionFactory& operator=(ActionFactory&&) = delete;
    ~ActionFactory() = default;

    /**
     * @brief Registers an action
     *
     * Registers an action as being available for configuration use. The action
     * is registered by its name and a function used to create the action
     * object. An action fails to be registered when another action of the same
     * name has already been registered. Actions with the same name would cause
     * undefined behavior, therefore are not allowed.
     *
     * Actions are registered prior to starting main().
     *
     * @param[in] name - Name of the action to register
     *
     * @return The action was registered, otherwise an exception is thrown.
     */
    template <typename T>
    static bool regAction(const std::string& name)
    {
        auto it = actions.find(name);
        if (it == actions.end())
        {
            actions[name] = &createAction<T>;
        }
        else
        {
            log<level::ERR>(
                fmt::format("Action '{}' is already registered", name).c_str());
            throw std::runtime_error("Actions with the same name found");
        }

        return true;
    }

    /**
     * @brief Gets a registered action's object
     *
     * Gets a registered action's object of a given name from the JSON
     * configuration data provided.
     *
     * @param[in] name - Name of the action to create/get
     * @param[in] jsonObj - JSON object for the action
     *
     * @return Pointer to the action object.
     */
    static std::unique_ptr<ActionBase> getAction(const std::string& name,
                                                 const json& jsonObj)
    {
        auto it = actions.find(name);
        if (it != actions.end())
        {
            return it->second(jsonObj);
        }
        else
        {
            // Construct list of available actions
            auto acts = std::accumulate(
                std::next(actions.begin()), actions.end(),
                actions.begin()->first, [](auto list, auto act) {
                    return std::move(list) + ", " + act.first;
                });
            log<level::ERR>(
                fmt::format("Action '{}' is not registered", name).c_str(),
                entry("AVAILABLE_ACTIONS=%s", acts.c_str()));
            throw std::runtime_error("Unsupported action name given");
        }
    }

  private:
    /* Map to store the available actions and their creation functions */
    static inline std::map<
        std::string, std::function<std::unique_ptr<ActionBase>(const json&)>>
        actions;
};

/**
 * @class ActionRegister - Registers an action class
 *
 * Base action registration class that is extended by an action object so
 * that action is registered and available for use.
 */
template <typename T>
class ActionRegister
{
  public:
    ActionRegister(const ActionRegister&) = delete;
    ActionRegister(ActionRegister&&) = delete;
    ActionRegister& operator=(const ActionRegister&) = delete;
    ActionRegister& operator=(ActionRegister&&) = delete;
    virtual ~ActionRegister() = default;
    ActionRegister()
    {
        // Templates instantiated when used, need to assign a value
        // here so the compiler doesnt remove it
        registered = true;
    }

  private:
    /* Register actions in the factory */
    static inline bool registered = ActionFactory::regAction<T>(T::name);
};

} // namespace phosphor::fan::control::json