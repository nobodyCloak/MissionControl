/*
 * Copyright (c) 2020 ndeadly
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "controller_management.hpp"
#include <stratosphere.hpp>
#include <memory>
#include <mutex>
#include <vector>
#include <cstring>

namespace ams::controller {

    namespace {

        const std::string official_npad_names[] = {
            "Joy-Con",
            "Pro Controller",
            "Lic Pro Controller",
            "NES Controller",
            "HVC Controller",
            "SNES Controller",
            "NintendoGamepad",
        };

        constexpr auto cod_major_peripheral  = 0x05;
        constexpr auto cod_minor_gamepad     = 0x08;
        constexpr auto cod_minor_joystick    = 0x04;
        constexpr auto cod_minor_keyboard    = 0x40;

        os::Mutex g_controller_lock(false);
        std::vector<std::unique_ptr<SwitchController>> g_controllers;

        inline bool bdcmp(const bluetooth::Address *addr1, const bluetooth::Address *addr2) {
            return std::memcmp(addr1, addr2, sizeof(bluetooth::Address)) == 0;
        }

    }

    ControllerType Identify(const bluetooth::DevicesSettings *device) {

        if (IsOfficialSwitchControllerName(device->name))
            return ControllerType_Switch;

        for (auto hwId : WiiController::hardware_ids) {
            if ( (device->vid == hwId.vid) && (device->pid == hwId.pid) ) {
                return ControllerType_Wii;
            }
        }

        for (auto hwId : Dualshock4Controller::hardware_ids) {
            if ( (device->vid == hwId.vid) && (device->pid == hwId.pid) ) {
                return ControllerType_Dualshock4;
            }
        }

        for (auto hwId : DualsenseController::hardware_ids) {
            if ( (device->vid == hwId.vid) && (device->pid == hwId.pid) ) {
                return ControllerType_Dualsense;
            }
        }

        for (auto hwId : XboxOneController::hardware_ids) {
            if ( (device->vid == hwId.vid) && (device->pid == hwId.pid) ) {
                return ControllerType_XboxOne;
            }
        }

        for (auto hwId : OuyaController::hardware_ids) {
            if ( (device->vid == hwId.vid) && (device->pid == hwId.pid) ) {
                return ControllerType_Ouya;
            }
        }

        for (auto hwId : GamestickController::hardware_ids) {
            if ( (device->vid == hwId.vid) && (device->pid == hwId.pid) ) {
                return ControllerType_Gamestick;
			}
		}

        for (auto hwId : GemboxController::hardware_ids) {
            if ( (device->vid == hwId.vid) && (device->pid == hwId.pid) ) {
                return ControllerType_Gembox;
            }
        }
				
        for (auto hwId : IpegaController::hardware_ids) {
            if ( (device->vid == hwId.vid) && (device->pid == hwId.pid) ) {
                return ControllerType_Ipega;
            }
        }  

        for (auto hwId : XiaomiController::hardware_ids) {
            if ( (device->vid == hwId.vid) && (device->pid == hwId.pid) ) {
                return ControllerType_Xiaomi;
            }
        }

        for (auto hwId : GamesirController::hardware_ids) {
            if ( (device->vid == hwId.vid) && (device->pid == hwId.pid) ) {
                return ControllerType_Gamesir;
            }
        }

        for (auto hwId : SteelseriesController::hardware_ids) {
            if ( (device->vid == hwId.vid) && (device->pid == hwId.pid) ) {
                return ControllerType_Steelseries;
            }
        }

        for (auto hwId : NvidiaShieldController::hardware_ids) {
            if ( (device->vid == hwId.vid) && (device->pid == hwId.pid) ) {
                return ControllerType_NvidiaShield;
			}
		}
		
        for (auto hwId : EightBitDoController::hardware_ids) {
            if ( (device->vid == hwId.vid) && (device->pid == hwId.pid) ) {
                return ControllerType_8BitDo;
            }
        }

        for (auto hwId : PowerAController::hardware_ids) {
            if ( (device->vid == hwId.vid) && (device->pid == hwId.pid) ) {
                return ControllerType_PowerA;
            }
        }

        for (auto hwId : MadCatzController::hardware_ids) {
            if ( (device->vid == hwId.vid) && (device->pid == hwId.pid) ) {
                return ControllerType_MadCatz;
            }
        }

        for (auto hwId : MocuteController::hardware_ids) {
            if ( (device->vid == hwId.vid) && (device->pid == hwId.pid) ) {
                return ControllerType_Mocute;
            }
        }

        for (auto hwId : RazerController::hardware_ids) {
            if ( (device->vid == hwId.vid) && (device->pid == hwId.pid) ) {
                return ControllerType_Razer;
            }
        }

        return ControllerType_Unknown;
    }

    bool IsAllowedDeviceClass(const bluetooth::DeviceClass *cod) {
        return ((cod->cod[1] & 0x0f) == cod_major_peripheral) &&
               (((cod->cod[2] & 0x0f) == cod_minor_gamepad) || ((cod->cod[2] & 0x0f) == cod_minor_joystick) || ((cod->cod[2] & 0x40) == cod_minor_keyboard));
    }

    bool IsOfficialSwitchControllerName(const std::string& name) {
        for (auto n : official_npad_names) {
            if (name.rfind(n, 0) == 0)
                return true;
        }

        return false;
    }

    void AttachHandler(const bluetooth::Address *address) {
        std::scoped_lock lk(g_controller_lock);

        bluetooth::DevicesSettings device;
        R_ABORT_UNLESS(btdrvGetPairedDeviceInfo(*address, &device));

        switch (Identify(&device)) {
            case ControllerType_Switch:
                g_controllers.push_back(std::make_unique<SwitchController>(address));
                break;
            case ControllerType_Wii:
                g_controllers.push_back(std::make_unique<WiiController>(address));
                break;
            case ControllerType_Dualshock4:
                g_controllers.push_back(std::make_unique<Dualshock4Controller>(address));
                break;
            case ControllerType_Dualsense:
                g_controllers.push_back(std::make_unique<DualsenseController>(address));
                break;
            case ControllerType_XboxOne:
                g_controllers.push_back(std::make_unique<XboxOneController>(address));
                break;
            case ControllerType_Ouya:
                g_controllers.push_back(std::make_unique<OuyaController>(address));
                break;
            case ControllerType_Gamestick:
                g_controllers.push_back(std::make_unique<GamestickController>(address));
				break;
            case ControllerType_Gembox:
                g_controllers.push_back(std::make_unique<GemboxController>(address));
                break;
            case ControllerType_Ipega:
                g_controllers.push_back(std::make_unique<IpegaController>(address));
				break;
            case ControllerType_Xiaomi:
                g_controllers.push_back(std::make_unique<XiaomiController>(address));
                break;
            case ControllerType_Gamesir:
                g_controllers.push_back(std::make_unique<GamesirController>(address));
				break;
            case ControllerType_Steelseries:
                g_controllers.push_back(std::make_unique<SteelseriesController>(address));
                break;
            case ControllerType_NvidiaShield:
                g_controllers.push_back(std::make_unique<NvidiaShieldController>(address));
				break;
            case ControllerType_8BitDo:
                g_controllers.push_back(std::make_unique<EightBitDoController>(address));
                break;
            case ControllerType_PowerA:
                g_controllers.push_back(std::make_unique<PowerAController>(address));
                break;
            case ControllerType_MadCatz:
                g_controllers.push_back(std::make_unique<MadCatzController>(address));
                break;
            case ControllerType_Mocute:
                g_controllers.push_back(std::make_unique<MocuteController>(address));
                break;
            case ControllerType_Razer:
                g_controllers.push_back(std::make_unique<RazerController>(address));
                break;
            default:
                g_controllers.push_back(std::make_unique<UnknownController>(address));
                break;
        }

        g_controllers.back()->Initialize();
    }

    void RemoveHandler(const bluetooth::Address *address) {
        std::scoped_lock lk(g_controller_lock);

        for (auto it = g_controllers.begin(); it < g_controllers.end(); ++it) {
            if (bdcmp(&(*it)->Address(), address)) {
                g_controllers.erase(it);
                return;
            }
        }
    }

    SwitchController *LocateHandler(const bluetooth::Address *address) {
        std::scoped_lock lk(g_controller_lock);

        for (auto it = g_controllers.begin(); it < g_controllers.end(); ++it) {
                if (bdcmp(&(*it)->Address(), address)) {
                    return (*it).get();
                }
        }

        return nullptr;
    }

}
