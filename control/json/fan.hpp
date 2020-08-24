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

#include "config_base.hpp"

#include <nlohmann/json.hpp>
#include <sdbusplus/bus.hpp>

namespace phosphor::fan::control::json
{

using json = nlohmann::json;

/**
 * @class Fan - Represents a configured fan control fan object
 *
 * A fan object contains the configured attributes for a fan within the system
 * that will be controlled by the fan control application. These configuration
 * attributes include, but are not limited to, the cooling zone in which the
 * fan is included, what sensors make up the fan, the target interface to be
 * used in setting a speed, and any profiles(OPTIONAL) the fan should be
 * included in.
 *
 * (When no profile for a fan is given, the fan defaults to always be included)
 *
 */
class Fan : public ConfigBase
{
  public:
    /* JSON file name for fans */
    static constexpr auto confFileName = "fans.json";

    Fan() = delete;
    Fan(const Fan&) = delete;
    Fan(Fan&&) = delete;
    Fan& operator=(const Fan&) = delete;
    Fan& operator=(Fan&&) = delete;
    ~Fan() = default;

    /**
     * Constructor
     * Parses and populates a zone fan from JSON object data
     *
     * @param[in] bus - sdbusplus bus object
     * @param[in] jsonObj - JSON object
     */
    Fan(sdbusplus::bus::bus& bus, const json& jsonObj);

    /**
     * @brief Get the list of profiles
     *
     * @return List of profiles this fan belongs to
     */
    inline const auto& getProfiles() const
    {
        return _profiles;
    }

    /**
     * @brief Get the zone
     *
     * @return Zone this fan belongs in
     */
    inline const auto& getZone() const
    {
        return _zone;
    }

    /**
     * @brief Get the list of sensors
     *
     * @return List of sensors with `Target` property
     */
    inline const auto& getSensors() const
    {
        return _sensors;
    }

    /**
     * @brief Get the sensors' interface
     *
     * @return Interface containing `Target` to use on sensors
     */
    inline const auto& getInterface() const
    {
        return _interface;
    }

  private:
    /* The sdbusplus bus object */
    sdbusplus::bus::bus& _bus;

    /**
     * Profiles this fan belongs to (OPTIONAL).
     * Otherwise always include this fan in the zone
     * it belongs to when no profiles given
     */
    std::vector<std::string> _profiles;

    /* The zone this fan belongs to */
    std::string _zone;

    /**
     * Sensors containing the `Target` property on
     * dbus that make up the fan
     */
    std::vector<std::string> _sensors;

    /**
     * Interface containing the `Target` property
     * to use in controlling the fan's speed
     */
    std::string _interface;
};

} // namespace phosphor::fan::control::json
