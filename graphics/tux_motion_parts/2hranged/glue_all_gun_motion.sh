
gluem -n -d 16 -i iso_boots1
gluem -n -d 16 -i iso_robe
gluem -n -d 16 -i iso_torso
gluem -n -d 16 -i iso_head
gluem -n -d 16 -i iso_weaponarm
gluem -n -d 16 -i iso_armour1
gluem -n -d 16 -i iso_helm1
gluem -n -d 16 -i iso_feet

cd ../weapons
gluem -n -d 16 -i iso_gun1
cd ../shields
gluem -n -d 16 -i iso_shieldarm_gun
cd ../gun_motion


echo "All the gun_motion files should be glued properly now."
echo " "
