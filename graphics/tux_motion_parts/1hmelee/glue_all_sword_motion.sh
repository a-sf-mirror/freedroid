cd feet
gluem -n -d 16 -i iso_feet
gluem -n -d 16 -i iso_boots1

cd ../head
gluem -n -d 16 -i iso_head
gluem -n -d 16 -i iso_helm1

cd ../torso
gluem -n -d 16 -i iso_torso
gluem -n -d 16 -i iso_armour1
gluem -n -d 16 -i iso_robe

cd ../shield
gluem -n -d 16 -i iso_shieldarm

cd ../weaponarm
gluem -n -d 16 -i iso_weaponarm

cd ../sword_motion

echo "All the sword_motion files for animated Tux parts should be glued properly now."
echo " "
