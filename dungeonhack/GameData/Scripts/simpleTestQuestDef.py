
player = getPlayer()

def OnQuestStart():
  startLocTemp = player.getCurrentLoc()
  questDunTemp = getFreeDungeonLoc()

  this.setQuestVariable("StartLocID",startLocTemp.getLocID())
  this.setQuestVariable("DungeonLocID",questDunTemp.getLocID())

  trappedMage = this.addQuestMonster("Zombie",questDunTemp)
  trappedMage.setAggroScriptFunction("onMageAttack")
  trappedMage.setDieScriptFunction("onMageDeath")

  this.addQuestMonster("Zombie",startLocTemp)
  zombieLeader = this.addQuestMonster("Zombie",startLocTemp)
  spiderMonster = this.addQuestMonster("Spider",startLocTemp)

  spiderMonster.setDieScriptFunction("onSpiderKingDeath")
  zombieLeader.setAggroScriptFunction("onAggro")


def OnQuestEnd():
  showMessage("Congratulations, you've completed the first quest ever in Dungeon Hack! You win a scone from your local bakery, you just need to go order and pay for it to redeem your prize!")
  addHudMessage("Watch our further development at forums.dfworkshop.net")
  addHudMessage("Thank's for playing!")


def OnQuestChangeStage(newStage):
  currentLocation = player.getCurrentLoc()
  dungeonLocation = getLocFromID(this.getQuestVariable("DungeonLocID"))

  if newStage == 1:
    showYesNoMessage("A voice speaking right into your head stops you in your tracks: 'Help me! I was once a powerfull mage but now I am sending you this message with the last of my power. I am trapped in the " + dungeonLocation.getName() + " nearby... will you help me?'")
    pressed = getLastButtonPressed()

    onAcceptDecline(pressed)
    addHudMessage("The voice fades away as quickly as it came.")

  elif newStage == 2:
    showMessage("As you leave, You hear some strange noises coming from back inside...")

  elif newStage == 3:
    showMessage("This zombie seems to be trying to say something to you as it shuffles towards you. It says 'Yeeaarrghh.'")

  elif newStage == 4:
    showMessage("The voice returns: 'I can feel you getting closer, please find me...'")

  elif newStage == 5:
    showMessage("Yes! You've made it. I am stuck here in limbo, please help release me. Be carefull as my mind cannot control my body anymore and it has a hunger for the living.'")

  elif newStage == 6:
    showMessage("Here I am, a great mage now reduced to being trapped in a decaying, uncontrollable body. I wanted immortality, but made a grave mistake. Please destroy my body, so my soul may be free.")

  elif newStage == 10:
    showMessage("Thank you for releasing me from my immortal prison. Sadly, I have nothing to give you as the programmer hasn't gotten to that part yet. What a pity. Oh well, off to the afterlife with me then.")
    this.finishQuest()


def onAcceptDecline(choice):
  dungeonLocation = getLocFromID(this.getQuestVariable("DungeonLocID"))
  if choice == 1:
      showMessage("Your actions will not go unrewarded. My strength is fading, I cannot hold this link to you any longer... I will contact you again when you reach the " + dungeonLocation.getName() + ".")
      showMessage("Oh! I see that you are unarmed. It is dangerous to go alone, take this!")
      player.createWeapon("shortsword")

  elif choice == 2:
      showMessage("Then I fear I am beyond help.")


def OnLocLoad():
  currentLocation = player.getCurrentLoc()

  if currentLocation.getLocID() == this.getQuestVariable("DungeonLocID"):
    if this.getStage() < 4:
      this.changeStage(4)

  if currentLocation.getLocID() == this.getQuestVariable("StartLocID"):
    if this.getStage() == 1:
      this.changeStage(2)


def OnLocInteriorLoad():
  currentLocation = player.getCurrentLoc()

  if currentLocation.getLocID() == this.getQuestVariable("DungeonLocID"):
    if this.getStage() < 5:
      this.changeStage(5)


def OnUpdate(TimeDelta):
  randomNumber = random.randint(1,30)

  if randomNumber == 2:
    addHudMessage("You feel that something is watching you")


def onSpiderKingDeath():
  showMessage("The giant spider finally convulses and dies, but it leaves no clues of it's origin.")


def onAggro():
  if this.getStage() < 3:
    this.changeStage(3)


def onMageAttack():
  if this.getStage() < 6:
    this.changeStage(6)


def onMageDeath():
  this.changeStage(10)
