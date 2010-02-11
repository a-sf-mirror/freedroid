
gluem -n -d 16 -i iso_boots1
gluem -n -d 16 -i iso_robe
gluem -n -d 16 -i iso_torso
gluem -n -d 16 -i iso_weaponarm
gluem -n -d 16 -i iso_armour1
gluem -n -d 16 -i iso_helm1
gluem -n -d 16 -i iso_feet

cd ../weapons
gluem -n -d 16 -i iso_mace
gluem -n -d 16 -i iso_head
gluem -n -d 16 -i iso_sword
gluem -n -d 16 -i iso_iron_pipe
cd ../shields
gluem -n -d 16 -i iso_heavy_shield
gluem -n -d 16 -i iso_shieldarm
gluem -n -d 16 -i iso_buckler
gluem -n -d 16 -i iso_standard_shield
gluem -n -d 16 -i iso_riot_shield
cd ../sword_motion

echo "All the sword_motion files should be glued properly now."
echo " "
