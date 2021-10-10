player = getPlayer()
thisQuest = getQuest("testquest")

if thisQuest.isNull() == true:
  addHudMessage("Quest Error!")


addHudMessage("Ran quest file.")

def OnQuestStart():
  addHudMessage("Quest Started.")

  questStartLoc = player.getCurrentLoc()
  dungeonLoc = getFreeDungeonLoc()

  thisQuest.setQuestVariable("questStartLocation",questStartLoc.getLocID())
  thisQuest.setQuestVariable("questDungeonLocation",dungeonLoc.getLocID())

  addHudMessage("Start location: " + questStartLoc.name)
  addHudMessage("Dungeon location: " + dungeonLoc.name)


def OnQuestEnd():
  addHudMessage("Quest ended.")


def OnQuestChangeStage(newStage):
  if newStage == 1:

    dungeonLocation = getLocFromID(thisQuest.getQuestVariable("questDungeonLocation"))
    questStartLocation = getLocFromID(thisQuest.getQuestVariable("questStartLocation"))

    showMessage("Quest Objective: Go to the " + dungeonLocation.name + ". Return to the " + questStartLocation.name + " when done.")
