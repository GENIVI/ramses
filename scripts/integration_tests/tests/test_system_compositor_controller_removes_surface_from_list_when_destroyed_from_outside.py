#  -------------------------------------------------------------------------
#  Copyright (C) 2017 BMW Car IT GmbH
#  -------------------------------------------------------------------------
#  This Source Code Form is subject to the terms of the Mozilla Public
#  License, v. 2.0. If a copy of the MPL was not distributed with this
#  file, You can obtain one at https://mozilla.org/MPL/2.0/.
#  -------------------------------------------------------------------------
from tests.system_compositor_controller_base import system_compositor_controller_base
from ramses_test_framework.ramses_test_extensions import ensureSystemCompositorRoundTrip


# Tests if the system compositor controller removes a surface from it's list of known surfaces, when the surface is destroyed
# from an outside applciation.
class TestSystemCompositorController(system_compositor_controller_base.SystemCompositorControllerBase):

    def impl_test(self):
        # Precondition: renderer and gears No. 1 and 2 visible
        self.renderer.send_ramsh_command("screct {0} 900 0 384 384".format(self.testSurfaceIVIIds["wlClient1"]), waitForRendererConfirmation=True)
        self.renderer.send_ramsh_command("scv {0} 1".format(self.testSurfaceIVIIds["wlClient1"]), waitForRendererConfirmation=True)
        self.renderer.send_ramsh_command("scv {0} 1".format(self.testSurfaceIVIIds["wlClient2"]), waitForRendererConfirmation=True)
        ensureSystemCompositorRoundTrip(self.renderer, self.testSurfaceIVIIds["wlClient2"])
        self.validateScreenshot(self.renderer, "scc_white_gear_left_red_gear_right_and_cube.png", useSystemCompositorForScreenshot=True)

        # Destroy surface of ivi-gears No. 1
        self.target.ivi_control.destroySurface(self.testSurfaceIVIIds["wlClient1"])
        self.target.ivi_control.flush()
        self.expectedSurfaceIds.remove("{0}".format(self.testSurfaceIVIIds["wlClient1"]))
        self.assertTrue(self.wait_on_surfaces_beeing_registered_in_scc())

        # Postcondition: renderer and gears No. 2 visible
        self.validateScreenshot(self.renderer, "scc_white_gear_left_and_cube.png", useSystemCompositorForScreenshot=True)
