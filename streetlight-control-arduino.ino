/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published 
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "relay.h"
#include "lora.h"
#include "debug.h"
#include "message.h"
#include "gps.h"
#include "automation.h"
#include "config/local.h"

s_configuration global_config = {
    .relayModes = 0,
    .timeOn = 0xff,
    .timeOff = 0xff,
    .outageOn = 0xff,
    .outageOff = 0xff,
};

unsigned long lastAutomationRun = millis();
bool automationEnabled = false;

void setup()
{
  debug_setup();

  LOG_MSG("Starting... [%s]\n", __NAME);

  pinMode(MAINTENANCE_PIN, INPUT_PULLUP);

  gps_setup();

  relay_setup();

  lora_start();
}

void loop()
{
  if (automationEnabled && millis() - lastAutomationRun > 5000)
  {
    lastAutomationRun = millis();

    LOG_MSG("Check automation\n");

    if (checkAutomation(&global_config))
    {
      lora_send_immediately();
    }
  }

  lora_once();

  //@TODO check time since last message exchange (in case tx message was not received, there will be no scheduled job)
}
