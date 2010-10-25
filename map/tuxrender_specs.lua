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
  standing_keyframe = 15,

  attack_duration = 0.55,
  attack_first_keyframe = 1,
  attack_last_keyframe = 14,

  walk_distance = 1.0,
  walk_first_keyframe  = 15,
  walk_last_keyframe = 24,

  run_distance = 10.0/3.0,
  run_first_keyframe = 25,
  run_last_keyframe = 34
}
