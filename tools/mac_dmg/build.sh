appdmg full.json $1
hdiutil unflatten $1
Rez -a sla.r -o $1
hdiutil flatten $1
