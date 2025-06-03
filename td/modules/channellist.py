WIDGETS = iop.Widgets
KEYFRAMER = parent.Keyframer


class WidgetExt(WIDGETS.Widget):
	"""
	WidgetExt description
	"""
	def __init__(self, ownerComp):
		super().__init__(ownerComp)
		self.listComp = ownerComp.op('list')

	def SetValue(self, element, value):
		KEYFRAMER.OnChannelListSetValue(element, value)
		
	def Refresh(self, channels_display, channelNames):
		self.listComp.par.rows = len(channelNames)
		self.listComp.par.Labels.expr = channelNames
		run("args[0](args[1])", self.refresh, channels_display,
			delayFrames=1)

	def refresh(self, channels_display):
		for i, display in enumerate(channels_display):
			if i < len(self.listComp.Toggles):
				self.listComp.Toggles[i][0] = display
				self.listComp.cellAttribs[i, 0].bgColor = [
					self.listComp.Settings.Cellcolor, 
					self.listComp.Settings.Cellselectcolor
				][int(self.listComp.Toggles[i][0])]
				
	def OnRSelectOffToOn(self, startrow, startcol, startcoords, 
					endrow, endcol, endcoords, start, end):
		chanName = self.listComp.cellAttribs[endrow, 0].text
		KEYFRAMER.OpenContextMenu(self.ownerComp, chanName)
		pass
		
	def StartEditCell(self, chanName):
		# self.row = KEYFRAMER.ChannelNames.val.index(chanName)
		self.listComp.StartEditCell(self.row, 0)
		
	def OnEditCell(self, row, col, value):
		# chanName = KEYFRAMER.ChannelNames.val[row]
		KEYFRAMER.RenameChannel(chanName, value)

	def OnMSelectOffToOn(self, startrow, startcol, startcoords, 
						endrow, endcol, endcoords, start, end):
		self.mSelectStartRow = startrow

	def OnMSelectOnToOff(self, startrow, startcol, startcoords, 
						endrow, endcol, endcoords, start, end):

		if endrow != None and endrow != -1:	
			KEYFRAMER.ReorderChannels(self.mSelectStartRow, endrow)
		
				

