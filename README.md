# TunnelMan
CS32 Final project.

This is a game where a "TunnelMan" sprite is controlled by the player. The player can move around an oil field to dig through Earth, with the goal to collect all (initially hidden) barrels of oil.
Some game objects include boulders which the TunnelMan cannot move through and can fall if the earth beneath them are removed, gold nuggets that the TunnelMan can collect and use to bribe protesters, water pools that randomly appear to refill the TunnelMan's squirt gun, and sonar kits that will reveal things in a certain radius around the player.

The primary antagonists are "Protesters", which are an AI that walk around and run towards the player if they see the TunnelMan. The player can stun protesters with their squirt gun or bribe them with gold nuggets, to slow them down. There are hardcore protesters as well which will track the player if within a certain radius and navigate through paths to get to them. Hardcore protesters cannot be bribed, either.
Upon being sufficiently "annoyed" by the squirt gun, a falling boulder, or bribed by a dropped gold nugget, a protester will navigate to the exit of the oil field.

This game has visual sprites with animations and gameplay audio.

## Controls
Arrow keys - move
Space - Fire squirt gun
Z - Activate sonar
Tab - Drop gold onto oil field (only works if TunnelMan has gold in inventory)
