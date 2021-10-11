def OnActivate():
  showYesNoMessage("Hello traveler! Are you looking for a room? It's only 10 gold to stay the night.")
  choice = getLastButtonPressed()

  if choice == 1:
      showMessage("Good choice, you'll find our beds much more refreshing than the hard ground on your back.")
  elif choice == 2:
      showMessage("Maybe next time then. I'll be here if you change your mind.")
