player = getPlayer()

startLocation = None
questDungeon = None

def OnQuestStart():
  startLocation = player.getCurrentLoc()
  questDungeon = getFreeDungeonLoc()

  if questDungeon.isNull() == False and startLocation.isNull() == False:
    questDungeon.inUse = True
    showMessage("A voice speaking right into your head stops you in your tracks: 'Help me! I was once a powerfull mage but now I am sending you this message with the last of my power. I am trapped in the " + questDungeon.name + " nearby... please help me, I've been here for so long...'")
    addHudMessage("The voice fades away as quickly as it came.")


def onQuestEnd():
  addHudMessage("Quest ended.")


def OnLocLoad():
  currentLocation = player.getCurrentLoc()

  if questLocation.name == currentLocation.name:
    showMessage("The voice returns: 'I can feel you getting closer, please find me...'")


def OnLocInteriorLoad():
  currentLocation = player:getCurrentLoc()

  if questLocation.name == currentLocation.name:
    showMessage("Yes! You've made it. I am stuck here in limbo, please help release me. Be carefull as my mind cannot control my body anymore and it has a hunger for the living.'")
