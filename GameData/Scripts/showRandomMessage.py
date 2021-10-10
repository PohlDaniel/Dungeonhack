randNumber = random.randint(1,2)

if randNumber == 1:
  addHudMessage("You suddenly feel different somehow")
elif randNumber == 2:
  addHudMessage("The room seems to spin around you!")

playSound("SpellCast.wav",0.3)
