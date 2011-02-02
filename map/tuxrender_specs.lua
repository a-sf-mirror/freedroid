------------------------------------------------------------------------------
-- This file defines the animation and rendering of Tux.
------------------------------------------------------------------------------

------------------------------------------------------------------------------
-- Specification of Tux animations.
--
-- Keyframe numbering starts at 0.
--
-- 'duration' is the duration of the related animation, in seconds.
--
-- For walk and run animations, 'distance' is the number of tiles covered by
-- Tux during a whole sequence of animation.
--
-- Note: there is currently no standing animation is the image-archive file,
-- so we only use a single keyframe.
------------------------------------------------------------------------------

tux_animation {
  standing_keyframe = 24,

  attack_duration = 0.55,
  attack_first_keyframe = 0,
  attack_last_keyframe = 13,

  walk_distance = 1.2,
  walk_first_keyframe  = 14,
  walk_last_keyframe = 23,

  run_distance = 10.0/3.0,
  run_first_keyframe = 25,
  run_last_keyframe = 34
}

------------------------------------------------------------------------------
-- Global configuration of Tux rendering
--
-- motion_class_names: list of motion class names
--   Those names are used in the next section of this file to define the ordering
--   of Tux's parts rendering, and in freedroid.item_archetypes to associate a
--   motion class to a weapon.
--   The first motion_class is the one used when Tux has no weapon in hand.
------------------------------------------------------------------------------

tux_rendering_config {
	motion_class_names = { "sword_motion", "gun_motion" }
}

------------------------------------------------------------------------------
-- Specification of how to order Tux parts' during rendering
--
-- The definition is:
--
--   tux_ordering {
--     type        : one of the motion_class value - example: "sword_motion"
--     rotations   : comma-separated list of rotation values - example: { 0, 1, 2 }
--     phase_start : first phase to which this order applies (optional - default value: 0)
--     phase_end   : last phase to which this order applies (optional - default value: -1,
--                   which denotes the last phase number)
--     order       : one of the parts' order definition - example: part_order.FTHASW
--   }
--
-- It is thus possible to specify the same order for several rotations and several
-- animation phases.
--
-- When a specific order is needed for a specific phase, it has to be defined
-- *before* the less specific set. For example:
--
--   tux_ordering {
--     type = "gun_motion",
--     rotations = { 1, 2 },
--     phase_start = 4,
--     phase_end = 11,
--     order = part_order.FTHSWA
--   }
--
--   tux_ordering {
--     type = "gun_motion",
--     rotations = { 1, 2 },
--     order = part_order.FSTHWA
--   }
------------------------------------------------------------------------------

--------------------
-- "part_order" is the definition of a list of orders.
-- The name of an order has no real meaning.
-- Here we chose the first letter of each part (with A = weaponarm).
--
-- (Note: part_order is a Lua variable, internal to this script. It does not
--  call a table constructor).
--------------------

part_order = {
  FTSWAH = { "feet", "torso",     "shieldarm", "weapon",    "weaponarm", "head"      },
  FTHSWA = { "feet", "torso",     "head",      "shieldarm", "weapon",    "weaponarm" },
  FTHSAW = { "feet", "torso",     "head",      "shieldarm", "weaponarm", "weapon"    },
  FTSAWH = { "feet", "torso",     "shieldarm", "weaponarm", "weapon",    "head"      },
  FTSAHW = { "feet", "torso",     "shieldarm", "weaponarm", "head",      "weapon"    },
  FTASWH = { "feet", "torso",     "weaponarm", "shieldarm", "weapon",    "head"      },
  FTAWSH = { "feet", "torso",     "weaponarm", "weapon",    "shieldarm", "head"      },
  FTWASH = { "feet", "torso",     "weapon",    "weaponarm", "shieldarm", "head"      },
  FATWSH = { "feet", "weaponarm", "torso",     "weapon",    "shieldarm", "head"      },
  FATSWH = { "feet", "weaponarm", "torso",     "shieldarm", "weapon",    "head"      },
  FATHWS = { "feet", "weaponarm", "torso",     "head",      "weapon",    "shieldarm" },
  FSTHWA = { "feet", "shieldarm", "torso",     "head",      "weapon",    "weaponarm" },
  FSWTAH = { "feet", "shieldarm", "weapon",    "torso",     "weaponarm", "head"      },
  FSWATH = { "feet", "shieldarm", "weapon",    "weaponarm", "torso",     "head"      },
  FSTWAH = { "feet", "shieldarm", "torso",     "weapon",    "weaponarm", "head"      },
  FWATSH = { "feet", "weapon",    "weaponarm", "torso",     "shieldarm", "head"      },
  FWASTH = { "feet", "weapon",    "weaponarm", "shieldarm", "torso",     "head"      },
  FWTASH = { "feet", "weapon",    "torso",     "weaponarm", "shieldarm", "head"      },
  FWSTAH = { "feet", "weapon",    "shieldarm", "torso",     "weaponarm", "head"      },
  FAWTSH = { "feet", "weaponarm", "weapon",    "torso",     "shieldarm", "head"      },
  FATWSH = { "feet", "weaponarm", "torso",     "weapon",    "shieldarm", "head"      },
  WFASTH = { "weapon", "feet",    "weaponarm", "shieldarm", "torso",     "head"      }
}

--------------------
-- Sword animation
--------------------

tux_ordering {
  type = "sword_motion",
  rotations = { 0 },
  order = part_order.FTHSWA
}

tux_ordering {
  type = "sword_motion",
  rotations = { 1 },
  phase_start = 0,
  phase_end = 10,
  order = part_order.FTHSWA
}

tux_ordering {
  type = "sword_motion",
  rotations = { 1, 2, 3, 4, 5, 6 },
  order = part_order.FSTHWA
}

tux_ordering {
  type = "sword_motion",
  rotations = { 7 },
  order = part_order.FSTWAH
}

tux_ordering {
  type = "sword_motion",
  rotations = { 8 },
  order = part_order.FSWTAH
}

tux_ordering {
  type = "sword_motion",
  rotations = { 9, 10, 11 },
  order = part_order.FWATSH
}

tux_ordering {
  type = "sword_motion",
  rotations = { 12, 13, 14, 15 },
  order = part_order.FAWTSH
}

--------------------
-- Gun Animation
--------------------

tux_ordering {
  type = "gun_motion",
  rotations = { 0, 1, 2 },
  order = part_order.FTSWAH
}

tux_ordering {
  type = "gun_motion",
  rotations = { 3, 4 },
  order = part_order.FSTWAH
}

tux_ordering {
  type = "gun_motion",
  rotations = { 5, 6, 7 },
  order = part_order.FSWTAH
}

tux_ordering {
  type = "gun_motion",
  rotations = { 8 },
  order = part_order.FSWATH
}

tux_ordering {
  type = "gun_motion",
  rotations = { 9 },
  order = part_order.FWASTH
}

tux_ordering {
  type = "gun_motion",
  rotations = { 10 },
  order = part_order.FWATSH
}

tux_ordering {
  type = "gun_motion",
  rotations = { 11 },
  order = part_order.FAWTSH
}

tux_ordering {
  type = "gun_motion",
  rotations = { 12 },
  order = part_order.FATWSH
}

tux_ordering {
  type = "gun_motion",
  rotations = { 13, 14 },
  order = part_order.FATSWH
}

tux_ordering {
  type = "gun_motion",
  rotations = { 15 },
  order = part_order.FTSAWH
}
