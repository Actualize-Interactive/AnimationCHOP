vMath = op('vsu').module.VMath()
import traceback
import numpy as np
import colorsys
TD_CHOP_CHANNEL = Channel
from pprint import pprint

class KeyframerExt:
	def __init__(self, ownerComp):
		self.ownerComp = ownerComp
		self.iParsComp = ownerComp.op('iPars')
		self.default_animationChop = self.ownerComp.op('default_animation')

		self.curves_viewChop = iop.curves_view
		self.keyframes_viewChop = iop.keyframes_view
		self.segments_viewChop = iop.segments_view
		self.channels_viewChop = iop.channels_view
		self.animation_viewChop = iop.animation_view

		self.viewComp = ownerComp.op('view')
		self.keysViewComp = self.viewComp.op('keysView')
		self.keysPanel = self.keysViewComp.panel
		self.keysCamComp = self.keysViewComp.op('cam')
		self.keysTransformComp = self.keysViewComp.op('transform')
		self.keysRenderPickDat = self.keysViewComp.op('renderpick')
		self.switchMarqueeTop = self.keysViewComp.op('switchMarquee')

		self.MasterKeyboardChop = self.viewComp.op('MasterKeyboard')
		self.keysRenderPickComp = self.viewComp.op('timeGraph')
		self.channelControlsComp = self.viewComp.op('channelControls')
		self.channelListComp = self.channelControlsComp.op('channelList')
		self.editControlsComp = self.viewComp.op('editControls')
		self.keyframeControlsComp = self.editControlsComp.op('keyframeControls')
		self.contextMenuComp = self.viewComp.op('ContextMenu')	
		self.globalControlsComp = self.viewComp.op('globalControls')
		self.newAnimNameComp = self.globalControlsComp.op('newAnimName')
		self.animCompComp = self.globalControlsComp.op('animComp')	
		self.newChannelNamesComp = self.globalControlsComp.op('newChannelNames')

		self.kctrl = self.MasterKeyboardChop['kctrl']
		self.kalt = self.MasterKeyboardChop['kalt']
		self.kshift = self.MasterKeyboardChop['kshift']
		self.kd = self.MasterKeyboardChop['kd']
		self.kf = self.MasterKeyboardChop['kf']


		self.labelStartTx = 30
		self.navHomeMargins = [30, 10, 20, 20] # l,r,b,t
		self.navHomeMarginSums = [
			self.navHomeMargins[0] + self.navHomeMargins[1],
			self.navHomeMargins[2] + self.navHomeMargins[3],
		]
		self.undoStateChannelsName = 'Keyframer Channels State'
		self.undoStateAnimCompName = 'Keyframer Animation Comp State'
		self.undoNavName = 'Keyframer Navigate'

		self.selectFuncs = [
			self.SelectKey,
			self.SelectHandle, 
			self.SelectSegment
		]
		self.SetSelectedFuncs = {
			'frame': self.SetFrameSelectedKeyframes,
			'value': self.SetValueSelectedKeyframes,
			'inslope': self.SetInslopeSelectedKeyframes,
			'inaccel': self.SetInaccelSelectedKeyframes,
			'outslope': self.SetOutslopeSelectedKeyframes,
			'outaccel': self.SetOutAccelSelectedKeyframes,
			'function': self.SetFunctionSelectedKeyframes
		}
		self.nudgeDir = {
			'right': tdu.Position(1, 0, 0),
			'left': tdu.Position(-1, 0, 0), 
			'up': tdu.Position(0, 1, 0), 
			'down': tdu.Position(0, -1, 0)
		}
		self.contextMenuLookup = {
			'channelList': {
				'headerLabel': 'Edit Channel(s)',
				'menuItems': [
					{
						'label': 'Delete Channel',
						'func': self.DeleteChannelConfirm,
					},
					{
						'label': 'Delete Displayed Channels',
						'func': self.DeleteChannelsConfirm,
					},
					{
						'label': 'Rename Channel',
						'func': self.EditChannelName,
					}				
				]
			}
		}	

		self.init()


	@property
	def AnimationChop(self): return self.ownerComp.par.Animationchop.eval()
	@AnimationChop.setter
	def AnimationChop(self, value):
		if isinstance(value, AnimationCHOP):
			self.ownerComp.par.Animationchop = value
			self.animCompComp.UpdateView(value)

	@property
	def AnimationRangeStart(self):
		return self.AnimationChop.par.Range1.eval()
	@AnimationRangeStart.setter
	def AnimationRangeStart(self, value):
		self.AnimationChop.par.Range1 = value

	@property
	def AnimationRangeEnd(self):
		return self.AnimationChop.par.Range2.eval()
	@AnimationRangeEnd.setter
	def AnimationRangeEnd(self, value):
		self.AnimationChop.par.Range2 = value

	@property
	def KeysViewHorzRange(self):
		return [self.keysViewComp.par.Horzrange1.eval(), 
				self.keysViewComp.par.Horzrange2.eval()]
	@KeysViewHorzRange.setter
	def KeysViewHorzRange(self, value):
		self.keysViewComp.par.Horzrange1 = value[0]
		self.keysViewComp.par.Horzrange2 = value[1]
		self.curves_viewChop.par.Range1 = value[0]
		self.curves_viewChop.par.Range2 = value[1]

	@property
	def KeysViewVertRange(self):
		return [self.keysViewComp.par.Vertrange1.eval(), 
				self.keysViewComp.par.Vertrange2.eval()]
	@KeysViewVertRange.setter
	def KeysViewVertRange(self, value):
		self.keysViewComp.par.Vertrange1 = value[0]
		self.keysViewComp.par.Vertrange2 = value[1]

	@property
	def LengthSeconds(self):
		if self.AnimationChop is not None:
			animStart = self.AnimationRangeStart	
			animEnd = self.AnimationRangeEnd
			numFrames = animEnd - animStart
			return numFrames / self.AnimationChop.time.rate
		else:
			return 10		

	@property
	def channels(self):
		return self.AnimationChop.channels

	@property
	def curStateChannels(self):
		return self.ownerComp.storage.get('curStateChannels')
	@curStateChannels.setter
	def curStateChannels(self, value):
		self.ownerComp.storage['curStateChannels'] = value
		
	@property
	def prevStateChannels(self):
		return self.ownerComp.storage.get('prevStateChannels')
	@prevStateChannels.setter
	def prevStateChannels(self, value):
		self.ownerComp.storage['prevStateChannels'] = value

	@property
	def ChannelNames(self):
		return tdu.Dependency(list(self.AnimationChop.channel_names))

	@property
	def currentChannelIds(self):
		return [int(cell.val) for cell in self.channelsDat.col(1)[1:]]

	@property
	def marqueeSelecting(self):
		return bool(self.switchMarqueeTop.par.index)

	def getHandleRelXY(self, slope, radius):
		s = 1 if slope >= 0 else -1
		if slope != 0.0 and radius != 0.0:
			x = radius / math.sqrt(slope * slope + 1)
			slope = 1.0 / slope
			y = radius / (s * math.sqrt(slope * slope + 1))
		elif radius == 0.0 and slope != 0.0:
			x = 0
			y = 0
		else:
			x = radius
			y = 0.0
		return x, y

	def init(self, newAnimComp=False):
		self.updateViewKeySegment = None
		self.updateViewInHandleSegment = None
		self.updateViewOutHandleSegment = None
		self.startPickOP = None
		self.keysRenderPickDat.par.strategy = 'holdfirst'
		self.selectPos = tdu.Position()
		self.setPos = tdu.Position()
		self.begin_set_pos = tdu.Position()
		self.insertPos = tdu.Position()
		self.insidePos = tdu.Position()

		try:
			if newAnimComp:
				channelComps = self.ChannelsComp.findChildren()
				for channelComp in channelComps:
					if channelComp:
						channelComp.destroy()				
			self.SetChannels()
			self.curves_viewChop.unselect_all()
			self.AnimationChop.cook(force=True, recurse=True)
			if newAnimComp:
				self.keysNavHome()
				# self.updateKeysView(init=True)							
		except Exception as e:
			traceback.print_exc()

	def InitView(self):
		self.SetNavRange()
		self.updateKeysView()

	def Animationcomp(self):
		if self.AnimationChop is None:
			self.AnimationChop = self.defaultAnimationChop
		elif 'KEYFRAMER_ANIM_COMP' not in self.AnimationChop.tags:
			m = (f"The animationCOMP: {self.AnimationChop.path} is not 'Keyframer' "
				f"configured would you like to convert it?")
			confirm = ui.messageBox('Convert', m, 
									buttons=['Cancel', 'convert'])
			if confirm == 1:
				comp = self.convertToKeyframerAnimComp(self.AnimationChop)
				if comp is not None:
					self.AnimationChop = comp

				else:
					print(f"Unable to convert: {self.AnimationChop.path}")
					self.AnimationChop = self.defaultAnimationChop		
			else:
				self.AnimationChop = self.defaultAnimationChop
			

		self.init(newAnimComp=True)

	def SetChannels(self):
		# channels = {}
		# for i, row in enumerate(self.channelsDat.rows()[1:], start=1):
		# 	name = row[0].val
		# 	chan = Channel(self, name, int(row[1].val), i - 1)
		# 	channels[name] = chan
		# 	chan.channelComp.par.Colorr = row[6].val
		# 	chan.channelComp.par.Colorg = row[7].val
		# 	chan.channelComp.par.Colorb = row[8].val						


		self.refreshChannelList()

		self.updateKeysView(init=True)
		# self.GetKeyHandlesActive()

	def refreshChannelList(self):
		channels_display = self.channels_viewChop['display'].vals
		channel_names = self.AnimationChop.channel_names
		self.channelListComp.Refresh(channels_display, channel_names)


	def OnPickEvents(self, allEvents):
		for event in allEvents:
			# print(event)
			if event.selectStart:
				self.onPickSelectStart(event)
				if (self.keysPanel.lselect and not self.kalt):
					self.startPickOP = event.pickOp
					if event.pickOp == None:	
						if not self.kshift and not self.kctrl:
							self.curves_viewChop.unselect_all()
						self.marqueeSelectStart(event)
					elif (event.pickOp is not None 
								and not self.kshift
								and not self.itemIsSelected(event)):	
						self.curves_viewChop.unselect_all()				
						self.selectItem(event)
					elif event.pickOp is not None:			
						self.selectItem(event)
				elif self.keysPanel.lselect and self.kalt:
					if self.kctrl == 0:
						self.insertKeyInNearestCurve(event)
					else:
						self.insertKeys(event)		
			elif event.selectEnd:
				self.keysNavEnd(event)	
				self.onPickEnd(event)	
			if self.keysPanel.lselect:
				if (event.select and event.pickOp is not None 
								and event.pickOp == self.startPickOP
								and not self.marqueeSelecting):			
					if not self.kd and not self.kf:
						self.offset_selected(event)
					else:
						self.scaleKeys(event)
				elif event.select:
					self.marqueeSelect(event)
			elif self.keysPanel.mselect:
				self.keysNav(event)							
			if event.selectEnd:
				self.keysNavEnd(event)	
				self.onPickEnd				

	def Appendchannel(self):
		# par callback
		name = self.iParsComp.par.Channelname.eval()
		self.AppendChannel(name)

	def Deletechannel(self):
		# par callback
		name = self.iParsComp.par.Channelname.eval()
		self.DeleteChannel(name)

	def Insertkey(self):
		# par callback		
		x = self.iParsComp.par.Insertposx.eval()
		chanName = self.iParsComp.par.Channelname.eval()
		y = None
		expr = self.iParsComp.par.Insertfunction.eval()
		self.InsertKey(chanName, x, y, expr)

	def Deletekey(self):
		# par callback
		i = self.iParsComp.par.Selectkeyframe.eval()
		chanName = self.iParsComp.par.Channelname.eval()
		self.DeleteKey(chanName, i)

	def Defaultchancol(self):
		# par callback
		if self.iParsComp.par.Chancolmode.eval() == 'DEFAULT_COL':
			col = self.iParsComp.pars('Defaultchancol*')
			templatePars = self.channelTemplateComp.pars('Color*')
			templatePars[0].val = col[0]
			templatePars[1].val = col[1]
			templatePars[2].val = col[2]
			for i, chan in enumerate(self.Channels.values(), start=1):
				self.channelsDat[i, 6] = col[0]
				self.channelsDat[i, 7] = col[1]
				self.channelsDat[i, 8] = col[2]
				chan.channelComp.par.Colorr = col[0]
				chan.channelComp.par.Colorg = col[1]
				chan.channelComp.par.Colorb = col[2]

	def Initialize(self, templateComp=False):
		if templateComp:
			comp = self.defaultAnimationChop
		else:
			comp = self.AnimationChop
		confirm = ui.messageBox('Initialize', 
			(f"Are you sure? This will delete all existings data in: "
			f"{comp}"), buttons=['Cancel', 'Initialize'])
		if confirm == 1:
			if templateComp:
				self.AnimationChop = self.defaultAnimationChop
			self.DeleteAllChannels()
			self.updateKeysView(init=True)
			run("args[0]()", self.keysNavHome, delayFrames=1)				

	def AppendChannel(self, name, updateKeysView=True, startVal=None):
		if name == '' or name == 'name' or self.AnimationChop.has_channel(name):
			print(f"'{name}' is an invalid channel name.")
			return False
		else:
			channel = self.AnimationChop.create_channel(name)
			channel.create_keyframe(0.0, 0.0)
			channel.create_keyframe(1.0, 0.0)

	def DeleteChannel(self, name):
		if self.Channels.get(name):
			self.Channels.get(name).destroy()
		self.SetChannels()
		self.updateKeysView()

	def DeleteAllChannels(self):
		self.Channels = {}
		for channel in self.Channels.values():
			channel.destroy()
		self.channelsDat.clear(keepFirstRow=True)
		channelComps = self.ChannelsComp.findChildren()
		for channelComp in channelComps:
			if channelComp:
				channelComp.destroy()
		keysDats = self.keysComp.findChildren()
		for keysDat in keysDats:
			if keysDat:
				keysDat.destroy()
		self.refreshChannelList()

	def DeleteChannelConfirm(self, chanName):
		confirm = ui.messageBox('Delete Channel',
			f"Are you sure you would like to delete: {chanName}?",
			buttons = ['Cancel', 'Delete'])
		if confirm == 1:
			prevState = self.getAnimationCompState()
			self.DeleteChannel(chanName)
			curState = self.getAnimationCompState()
			ui.undo.startBlock(self.undoStateAnimCompName)
			ui.undo.addCallback(self.undoDeleteAppendChannels, 
								[prevState, curState])
			ui.undo.endBlock()

	def DeleteChannelsConfirm(self, chanName):
		confirm = ui.messageBox('Delete Channel',
			f"Are you sure you would like to delete all displayed channels?",
			buttons = ['Cancel', 'Delete'])
		if confirm == 1:
			chans = []
			for chan in self.Channels.values():
				if chan.display:
					chans.append(chan)
			prevState = self.getAnimationCompState()
			for chan in chans:
				self.DeleteChannel(chan.name)
			curState = self.getAnimationCompState()
			ui.undo.startBlock(self.undoStateAnimCompName)
			ui.undo.addCallback(self.undoDeleteAppendChannels, 
								[prevState, curState])
			ui.undo.endBlock()

	def getAnimationCompState(self):
		state = {
			'channelsDat': self.channelsDat.text,
			'keysComp': self.keysComp.saveByteArray()
		}
		return state

	def undoDeleteAppendChannels(self, isUndo, info):
		if isUndo:
			state = info[0]
		else:
			state = info[1]	
		if state is not None:	
			self.channelsDat.text = state['channelsDat']
			self.keysComp.destroy()
			self.AnimationChop.loadByteArray(state['keysComp'])
			self.SetChannels()
			self.updateKeysView(init=True)
			pass

	def EditChannelName(self, chanName):
		self.channelListComp.StartEditCell(chanName)

	def RenameChannel(self, chanName, newName):
		if newName in self.ChannelNames.val:
			ui.messageBox('Warning', 
				f"{newName} already exists! Please use another name.",
				buttons=['Close'])
			return
		prevState = self.getAnimationCompState()
		self.Channels[chanName].setName(newName)		
		run("args[0]()", self.SetChannels, delayFrames=1)
		run("args[0](args[1])", self.postRenameChannel, prevState, 
			delayFrames=2)

	def postRenameChannel(self, prevState):
		curState = self.getAnimationCompState()
		ui.undo.startBlock(self.undoStateAnimCompName)
		ui.undo.addCallback(self.undoDeleteAppendChannels, 
							[prevState, curState])
		ui.undo.endBlock()

	def Toolmode(self):
		index = 6 if self.iParsComp.par.Toolmode.eval() == 'draw' else 0
		self.keysViewComp.par.cursor = index

	def unSelectAll(self):
		self.keyframes_viewChop.unselect_all_keyframes()
		self.segments_viewChop.unselect_all_segments()
		self.segments_viewChop.unselect_all_start_handles()
		self.segments_viewChop.unselect_all_end_handles()
		self.channels_viewChop.unselect_all_channels()
		pass

	def transformPos(self, event):
		self.selectPos.x = event.u * self.keysViewComp.width
		self.selectPos.y = event.v * self.keysViewComp.height
		self.selectPos = self.keysTransformComp.worldTransform * self.selectPos
		return self.selectPos

	def SelectChannel(self, chan):
		if chan not in self.selectedChannels:
			self.selectedChannels.append(chan)

	def SelectAllKeys(self):
		indices = [i for i in range(self.keyframes_viewChop.numSamples)]
		self.keyframes_viewChop.select_keyframes(indices)		
		self.KeyframeControlsUpdateView(updateFunction=True)	

	def SelectAdjacentItem(self, dir):
		selected_keys = [i for i, val in enumerate(self.keyframes_viewChop['selected'].vals) if val]
		if len(selected_keys) == 1:
			if dir == 1: # right	
				new_idx = (selected_keys[0] + 1) % self.keyframes_viewChop.numSamples
			elif dir == -1: # left
				new_idx = (selected_keys[0] - 1) % self.keyframes_viewChop.numSamples
			self.curves_viewChop.unselect_all_keyframes()
			self.curves_viewChop.select_keyframes([new_idx])
		elif len(selected_keys) > 1:
			return
		else: # check if one handle is selected and step through handles if so
			selected_start_handles = [i for i, val in enumerate(self.segments_viewChop['selected_start_handles'].vals) if val]
			selected_end_handles = [i for i, val in enumerate(self.segments_viewChop['selected_end_handles'].vals) if val]
			if len(selected_start_handles) + len(selected_end_handles) == 1:
				self.curves_viewChop.unselect_all_start_handles()
				self.curves_viewChop.unselect_all_end_handles()
				if dir == 1: # right
					if len(selected_start_handles) == 1:
						# next handle is the end handle of this segment
						self.curves_viewChop.select_end_handles(selected_start_handles)
					else: # len(selected_end_handles) must be 1
						# next handle is the start handle of next segment
						self.curves_viewChop.select_start_handles([(selected_end_handles[0] + 1) % self.segments_viewChop.numSamples])
				elif dir == -1: # left
					if len(selected_end_handles) == 1:
						# next handle is the start handle of this segment
						self.curves_viewChop.select_start_handles(selected_end_handles)
					else: # len(selected_start_handles) must be 1
						# next handle is the end handle of previous segment
						self.curves_viewChop.select_end_handles([(selected_start_handles[0] - 1) % self.segments_viewChop.numSamples])

		self.KeyframeControlsUpdateView(updateFunction=True)	



	def NudgeItem(self, nudge):

		offset = tdu.Position(
			nudge.x / self.keysViewComp.width,
			nudge.y/ self.keysViewComp.height,
			0)

		if len(self.curves_viewChop.selected_keyframes()) > 0:
			self.curves_viewChop.offset_selected_keyframes(offset.x, offset.y)
		else:
			if len(self.curves_viewChop.selected_start_handles()) > 0 or len(self.curves_viewChop.selected_end_handles()) > 0:
				self.curves_viewChop.offset_selected_start_handles(offset.x, offset.y)
				self.curves_viewChop.offset_selected_end_handles(offset.x, offset.y)
		
		self.curves_viewChop.reset_begin_set_values()


	def InsertKey(self, chanName, x, y=None, expr='bezier()'):
		chan = self.Channels[chanName]
		if chan.display:
			x = math.floor(self.insertPos.x)
			if all([segment.x0 != x and segment.x1 != x
						for segment in chan.segments]):
				chan.insertKey(x, y, funcName=expr)

	def insertKeyInNearestCurve(self, event):
		self.insertPos.x = event.u * self.keysViewComp.width
		self.insertPos.y = event.v * self.keysViewComp.height
		self.insertPos = self.keysTransformComp.worldTransform * self.insertPos		
		lastSampleIndex = self.curves_viewChop.numSamples - 1
		x = self.insertPos.x
		i = max(0, 
			min(lastSampleIndex, math.floor((x - self.KeysViewHorzRange[0]) * self.curves_viewChop.rate)))
		nearestChopChan = None
		nearestValue = None
		for chopChan in self.curves_viewChop.chans():	
			if self.channels_viewChop['display'][chopChan.index]:	
				if nearestChopChan == None:
					nearestChopChan = chopChan
					nearestValue = chopChan[i]
				elif (abs(chopChan[i] - self.insertPos.y) 
						< abs(nearestChopChan[i] - self.insertPos.y)):
					nearestChopChan = chopChan
					nearestValue = chopChan[i]
		if nearestChopChan is not None:
			# print(f"index: {i}, nearestChopChan: {nearestChopChan.name}, nearestValue: {nearestValue} ")
			
			chan = self.AnimationChop.channels[nearestChopChan.index]
			chan.create_keyframe(x, nearestValue)
			
	def insertKeys(self, event):
		self.insertPos.x = event.u * self.keysViewComp.width
		self.insertPos.y = event.v * self.keysViewComp.height
		self.insertPos = self.keysTransformComp.worldTransform * self.insertPos	
		self.curves_viewChop.unselect_all()
		insertInfos = {}
		for chan in self.Channels.values():
			if chan.display:
				insertInfos[chan.name] = chan.insertKey(
												self.insertPos.x, None)
		return insertInfos

	def copyKeys(self):
		self.copiedKeys = {}
		offsetX = None
		for name, chan in self.Channels.items():
			if chan.display:		
				selKeys = sorted(chan.selectedKeys)
				for i, _id in enumerate(selKeys):
					if _id < chan.numSegments:
						segment = chan.segments[_id]
						if offsetX is None:
							offsetX = segment.x0
						else:
							offsetX = min(offsetX, segment.x0)
					else:
						segment = chan.segments[_id - 1]
						if offsetX is None:
							offsetX = segment.x1
						else:
							offsetX = min(offsetX, segment.x1)
		if offsetX is not None:
			for name, chan in self.Channels.items():
				if chan.display:
					self.copiedKeys[name] = []
					selKeys = sorted(chan.selectedKeys)
					for i, _id in enumerate(selKeys):
						if _id < chan.numSegments:
							segment = chan.segments[_id]
							x = segment.x0 - offsetX
							keyData = (segment.segmentType, x, segment.y0,
									segment.inslope, segment.inaccel,
									segment.outslope, segment.outaccel)
						else:
							segment = chan.segments[_id - 1]
							x = segment.x1 - offsetX
							keyData = (segment.segmentType, x, segment.y1,
									segment.inslope, segment.inaccel,
									segment.outslope, segment.outaccel)
						self.copiedKeys[name].append(keyData)
		pprint(self.copiedKeys)

	def cutKeys(self):
		self.copyKeys()
		self.DeleteSelectedKeys()

	def pasteKeys(self):
		if self.keysPanel.inside:
			self.insertPos.x = self.keysPanel.insideu * self.keysViewComp.width
			self.insertPos.y = self.keysPanel.insidev * self.keysViewComp.height
			self.insertPos = (self.keysTransformComp.worldTransform 
							* self.insertPos)
			insertIndices = {}			
			for chanName, chanKeysData in self.copiedKeys.items():
				if chanName in self.ChannelNames.val:
					chan = self.Channels[chanName]
					insertIndices[chan.name] = []
					if chan.display:
						for keyData in chanKeysData:
							x = self.insertPos.x + keyData[1]
							insertIndex = chan.insertKey(x, keyData[2], 
													funcName=keyData[0])
							insertIndices[chan.name].append(
								(insertIndex, *keyData[3:])
							)
					insertIndices[chan.name].sort()

			for chanName, insertData in insertIndices.items():
				chan = self.Channels[chanName]
				for data in insertData:
					segment = chan.segments[data[0]]
					if segment.hasHandles:
						segment.setInSlopeAccel(data[1], data[2])
						segment.setOutSlopeAccel(data[3], data[4])

	def drawKeyframes(self, event):
		# needs update to function properly
		self.curves_viewChop.unselect_all()
		channel = list(self.Channels.values())[0]
		if event.selectStart:
			self.drawPrevPos = self.transformPos(event)
			self.drawPos = self.drawPrevPos.copy()
			self.drawPrevVector = self.drawPrevPos - self.drawPos
			self.drawPrevVector.normalize()	
			self.drawVector = self.drawPrevVector.copy()
			insertIndex = channel.insertKey(self.drawPos.x, 
								self.drawPos.y,
								self.iParsComp.par.Insertfunction.eval())
		
		else: 
			self.drawPos = self.transformPos(event)
			self.drawVector = self.drawPrevPos - self.drawPos
			self.drawVector.normalize()
			dot = self.drawPrevVector.dot(self.drawVector)
			dotThreshold = .88
			# #if self.drawPrevVector != self.drawVector:
			if dot < dotThreshold:
				insertIndex = channel.insertKey(self.drawPos.x, 
								self.drawPos.y,
								self.iParsComp.par.Insertfunction.eval())
			self.drawPrevPos = self.drawPos.copy()
			self.drawPrevVector = self.drawVector.copy()
			self.drawPrevVector.normalize()		

	def DeleteKey(self, chanName, i):
		if self.Channels[chanName].numSegments > 1:
			self.Channels[chanName].deleteKey(i)

	def DeleteSelectedKeys(self):
		for n in range(self.keyframes_viewChop.numSamples, 0, -1):
			i = n - 1
			selected = self.keyframes_viewChop['selected'][i]
			if selected:
				channel_index = round(self.keyframes_viewChop['channel_index'][i])
				keyframe_index = round(self.keyframes_viewChop['keyframe_index'][i])
				channel = self.AnimationChop.channels[channel_index]
				channel.delete_keyframe(keyframe_index)

	def scaleKeys(self, event):	
		offset = get_item_offset = self.get_item_offset(event)
		scale_x = offset.x / self.keysTransformComp.par.sx
		scale_y = offset.y / self.keysTransformComp.par.sy

		# TODO: implement scale_selected_keys
		# self.curves_viewChop.scale_selected_keys(scale_x, scale_y)
		
		self.setLabelsTy()						

	def selectItem(self, event):
		geo = event.pickOp.parent()
		i = geo.par.Geotype.menuIndex
		# print("selectItem", i, event.instanceId, event.custom)
		if i < 3:
			instance_id = int(event.instanceId)
			indices = event.custom['indices']
			
			if i == 0:
				# print("select keyframe", instance_id, indices)
				self.keyframes_viewChop.select_keyframes([instance_id])
			elif i == 1:
				# print("select end handle", instance_id, indices)
				self.segments_viewChop.select_end_handles([instance_id])
			elif i == 2:
				# print("select start handle", instance_id, indices)
				self.segments_viewChop.select_start_handles([instance_id])


	def SelectKey(self, chan, _id, state):
		self.SelectChannel(chan)
		chan.selectKey(_id, state)
		if len(self.selectedChannels) > 0:
			if len(chan.selectedKeys) > 0:
				if _id < chan.numSegments:
					self.updateViewKeySegment = chan.segments[_id]
				else:
					self.updateViewKeySegment = (chan.segments[_id - 1], 1)
				self.KeyframeControlsUpdateView(updateFunction=True)

	def SelectHandle(self, chan, _id, state):
		self.SelectChannel(chan)
		chan.selectHandle(_id, state)
		if len(self.selectedChannels) > 0:
			if len(chan.selectedHandles) > 0:
				segmentIndex = math.floor(_id / 2.0)
				isOutHandle = _id % 2	
				if isOutHandle == 1:
					self.updateViewOutHandleSegment = chan.segments[segmentIndex]
				else:
					self.updateViewInHandleSegment = chan.segments[segmentIndex]
				self.KeyframeControlsUpdateView()

	def SelectSegment(self, chan, event):
		pos = self.transformPos(event)
		segment = chan.selectSegment(pos.x, 1)
		self.SelectChannel(chan)
		if segment is not None:
			self.updateViewKeySegment = segment
		self.KeyframeControlsUpdateView()

	def itemIsSelected(self, event):
		geo = event.pickOp.parent()
		geo = event.pickOp.parent()
		i = geo.par.Geotype.menuIndex
		if i < 3:
			instance_id = int(event.instanceId)
			if i == 0:
				return self.keyframes_viewChop['selected'][instance_id]
			elif i == 1:
				return self.segments_viewChop['selected_end_handles'][instance_id]
			elif i == 2:
				return self.segments_viewChop['selected_start_handles'][instance_id]

		return False

	def offset_selected(self, event):
		geo = event.pickOp.parent()
		i = geo.par.Geotype.menuIndex

		if i < 3:
			offset = self.get_item_offset(event)
			if self.kshift:
				offset.y = 0
			elif self.kctrl:
				offset.x = 0
			if i == 0:
				self.curves_viewChop.offset_selected_keyframes(offset.x, offset.y)
			elif i == 1:
				self.curves_viewChop.offset_selected_end_handles(offset.x, offset.y)
			elif i == 2:
				self.curves_viewChop.offset_selected_start_handles(offset.x, offset.y)

		self.setLabelsTy()
		self.KeyframeControlsUpdateView()

	def get_item_offset(self, event):
		new_position = tdu.Position(
			event.u * self.keysViewComp.width,
			event.v * self.keysViewComp.height, 0)
		new_position = self.keysTransformComp.worldTransform * new_position
		return new_position - self.begin_set_pos


	def onPickSelectStart(self, event):
		self.SetPrevStateChannels()
		# print("onPickSelectStart", event)
		self.begin_set_pos.x = event.u * self.keysViewComp.width
		self.begin_set_pos.y = event.v * self.keysViewComp.height
		self.begin_set_pos = self.keysTransformComp.worldTransform * self.begin_set_pos

		self.pickStartVals = {}
		self.pickStartVals['uv'] = (event.u, event.v)
		self.pickStartVals['shift'] = self.keysViewComp.panel.shift.val
		self.pickStartVals['ctrl'] = self.keysViewComp.panel.ctrl.val
		self.pickStartVals['alt'] = self.keysViewComp.panel.alt.val


	def onPickEnd(self, didAction=True):
		self.keyframeControlsComp.ActiveAll(True, True, True)	
		if self.switchMarqueeTop.par.index:	
			self.marqueeSelectEnd()	
		self.startSet = False
		self.startScale = False
		self.curves_viewChop.reset_begin_set_values()
		# if didAction:
		# 	self.SetCurStateChannels()	
		# for chan in self.Channels.values():
		# 	chan.displayHandles()
		# 	chan.setStartSetKeys()
		# self.keyframeControlsComp.ActiveAll(*self.GetKeyHandlesActive())

	def SetCurStateChannels(self, force=False):
		# channelsState = {key:chan.getState() for key, chan in self.Channels.items()}
		# keyframeControlsState = self.keyframeControlsComp.GetState()
		# state = {
		# 	'channelsDat': self.channelsDat.text,
		# 	'channels': channelsState,
		# 	'keyframeControls': keyframeControlsState
		# }
		# if state != self.curStateChannels or force:	
		# 	self.curStateChannels = state
		# 	ui.undo.startBlock(self.undoStateChannelsName)
		# 	ui.undo.addCallback(self.undoStateChannels, 
		# 						[dict(self.prevStateChannels), 
		# 						dict(self.curStateChannels)])
		# 	ui.undo.endBlock()
		pass

	def SetPrevStateChannels(self):
		self.prevStateChannels = dict(self.curStateChannels)

	def undoStateChannels(self, isUndo, info):
		if isUndo:
			state = info[0]
		else:
			state = info[1]	
		if state is not None:
			self.channelsDat.text = state['channelsDat']
			channelsState = state['channels']
			for key, chanData in channelsState.items():
				self.Channels[key].setState(chanData)
			self.SetChannels()
			for key, chanData in channelsState.items():
				chan = self.Channels[key]
				for selKey in chanData['selectedKeys']:
					chan.selectKey(selKey, 1)
					self.SelectChannel(chan)		
				if len(chanData['selectedHandles']) > 0:
					chan.selectHandles(chanData['selectedHandles'])
					self.SelectChannel(chan)					
				chan.setStartSetKeys()				
			self.SetPrevStateChannels()
			self.curStateChannels = dict(state)			
			self.refreshChannelList()
			self.updateKeysView(init=True)	

			keyHandlesActive = self.GetKeyHandlesActive()
			self.keyframeControlsComp.ActiveAll(*keyHandlesActive)
			self.KeyframeControlsUpdateView(updateFunction=True)	

	def marqueeSelectStart(self, event):
		pass
		self.keysRenderPickDat.par.strategy = 'select'
		self.keysViewComp.par.Marqueeselectstartu = event.u
		self.keysViewComp.par.Marqueeselectstartv = event.v
		self.switchMarqueeTop.par.index = 1

	def marqueeSelect(self, event):
		self.selectPos = self.transformPos(event)

	def marqueeSelectEnd(self):
		if self.marqueeSelecting:
			mCoords = [[self.begin_set_pos.x, self.selectPos.x], 
						[self.begin_set_pos.y, self.selectPos.y]]
			mCoords[0].sort()
			mCoords[1].sort()

			selected_indices = [] # sample_index
			for sample_index in range(self.keyframes_viewChop.numSamples):
				if (self.keyframes_viewChop['time'][sample_index] >= mCoords[0][0]
						and self.keyframes_viewChop['time'][sample_index] < mCoords[0][1]
						and self.keyframes_viewChop['value'][sample_index] >= mCoords[1][0]
						and self.keyframes_viewChop['value'][sample_index] < mCoords[1][1]
						and self.keyframes_viewChop['display'][sample_index]):
					selected_indices.append(sample_index)
			self.keyframes_viewChop.select_keyframes(selected_indices)
			# print(f"Selected keyframes: {selected_indices}")

			selected_start_handles = []
			selected_end_handles = []
			for sample_index in range(self.segments_viewChop.numSamples):
				if (self.segments_viewChop['start_handle_time'][sample_index] >= mCoords[0][0]
						and self.segments_viewChop['start_handle_time'][sample_index] < mCoords[0][1]
						and self.segments_viewChop['start_handle_value'][sample_index] >= mCoords[1][0]
						and self.segments_viewChop['start_handle_value'][sample_index] < mCoords[1][1]
						and self.segments_viewChop['display_start_handle'][sample_index]):
					selected_start_handles.append(sample_index)
				
				if (self.segments_viewChop['end_handle_time'][sample_index] >= mCoords[0][0]
						and self.segments_viewChop['end_handle_time'][sample_index] < mCoords[0][1]
						and self.segments_viewChop['end_handle_value'][sample_index] >= mCoords[1][0]
						and self.segments_viewChop['end_handle_value'][sample_index] < mCoords[1][1]
						and self.segments_viewChop['display_end_handle'][sample_index]):
					selected_end_handles.append(sample_index)

			self.segments_viewChop.select_start_handles(selected_start_handles)
			self.segments_viewChop.select_end_handles(selected_end_handles)

			self.curves_viewChop.par.Range1 = self.keysViewComp.par.Horzrange1
			self.curves_viewChop.par.Range2 = self.keysViewComp.par.Horzrange2			

		# self.GetKeyHandlesActive()
		self.KeyframeControlsUpdateView(updateFunction=True)

		self.keysRenderPickDat.par.strategy = 'holdfirst'
		self.switchMarqueeTop.par.index = 0		

	def OnKeyboardIn(self, key, character,
					alt, lAlt, rAlt, ctrl, lCtrl, rCtrl,
					shift, lShift, rShift, state, time, cmd, lCmd, rCmd):
		# TODO create lookup... 
		if state:
			
			if key == 'a' and ctrl:
				self.SetPrevStateChannels()	
				self.SelectAllKeys()
				self.SetCurStateChannels()		
			elif key == 'h':
				# self.SetPrevStateChannels()	
				self.keysNavHome()
				# self.SetCurStateChannels()		
			elif key == 't':
				self.SetPrevStateChannels()	
				self.toggleSelectedLockHandles()
				self.SetCurStateChannels()		
			elif key == 'c' and ctrl:
				self.SetPrevStateChannels()	
				self.copyKeys()
				self.SetCurStateChannels()		
			elif key == 'x' and ctrl:
				self.SetPrevStateChannels()	
				self.cutKeys()
				self.SetCurStateChannels()		
			elif key == 'v' and ctrl:
				self.SetPrevStateChannels()	
				self.pasteKeys()
				self.SetCurStateChannels()			
			elif key == 'delete':
				self.SetPrevStateChannels()	
				self.DeleteSelectedKeys()
				self.SetCurStateChannels()			
			elif key == 'backspace':
				self.SetPrevStateChannels()	
				self.DeleteSelectedKeys()
				self.SetCurStateChannels()		
			elif key == 'tab':
				self.SetPrevStateChannels()	
				self.SelectAdjacentItem(-1 if shift else 1)
				self.SetCurStateChannels()		
			elif key in ('right', 'left', 'up', 'down'):
				self.SetPrevStateChannels()	
				scale = 1.0
				if self.kshift:
					scale = 2.0
				elif self.kctrl:
					scale = 4.0
				elif self.kalt:
					scale = 8.0
				nudge = self.nudgeDir[key] * scale
				self.NudgeItem(nudge)	
				self.SetCurStateChannels()		
				
	def keysNavHome(self):
		xBounds = [None, None]
		yBounds = [None, None]
		chansDisplayed = False

		if chansDisplayed:
			xRange = (xBounds[1] - xBounds[0])
			yRange = (yBounds[1] - yBounds[0])
			xStart = xBounds[0]
			yStart = yBounds[0]
			if yRange == 0:
				yRange = 2
				yStart -= 1
		else:
			xRange = self.animation_viewChop['max_keyframe_time'] - self.animation_viewChop['min_keyframe_time']
			yRange = self.animation_viewChop['max_keyframe_value'] - self.animation_viewChop['min_keyframe_value']
			xStart = self.animation_viewChop['min_keyframe_time']
			yStart = self.animation_viewChop['min_keyframe_value']

		sx = xRange / (self.keysViewComp.width - self.navHomeMarginSums[0])
		tx = xStart - sx * self.navHomeMargins[0]
		sy = yRange / (self.keysViewComp.height - self.navHomeMarginSums[1])
		ty = yStart - sy * self.navHomeMargins[2]
	

		self.keysTransformComp.par.tx = tx
		self.keysTransformComp.par.ty = ty
		self.keysTransformComp.par.sx = sx
		self.keysTransformComp.par.sy = sy
		self.keysCamComp.par.tx = 0
		self.keysCamComp.par.ty = 0
		self.keysCamComp.par.px = 0
		self.keysCamComp.par.py = 0
		self.keysCamComp.par.sx = 1
		self.keysCamComp.par.sy = 1
		self.SetNavRange()

	def keysNavEnd(self, event):
		s, r, t = self.keysCamComp.worldTransform.decompose()
		self.keysTransformComp.par.tx = t[0]
		self.keysTransformComp.par.ty = t[1]
		self.keysTransformComp.par.sx = s[0]
		self.keysTransformComp.par.sy = s[1]
		self.keysCamComp.par.tx = 0
		self.keysCamComp.par.ty = 0
		self.keysCamComp.par.px = 0
		self.keysCamComp.par.py = 0
		self.keysCamComp.par.sx = 1
		self.keysCamComp.par.sy = 1

	def keysNav(self, event):
		u = event.u
		v = event.v
		startU = self.pickStartVals['uv'][0]
		startV = self.pickStartVals['uv'][1]
		if self.pickStartVals['shift']:
			pivotX = startU * self.keysViewComp.width
			pivotY = startV * self.keysViewComp.height
			self.keysCamComp.par.px = pivotX
			self.keysCamComp.par.py = pivotY
			relU = startU - u
			relV = startV - v
			su = 1.0 + relU
			sv = 1.0 + relV
			su *= su
			sv *= sv
			self.keysCamComp.par.sx = max(0.0, su)
			self.keysCamComp.par.sy = max(0.0, sv)	
		else:
			relX = vMath.expandValue(startU - u, 0.0, self.keysViewComp.width)
			relY = vMath.expandValue(startV - v, 0.0, self.keysViewComp.height)	
			self.keysCamComp.par.tx = relX
			self.keysCamComp.par.ty = relY

		self.SetNavRange()

	def GetKeysNavInsidePos(self):
		try:
			self.insidePos.x = self.keysPanel.insideu.val * self.keysViewComp.width
			self.insidePos.y = self.keysPanel.insidev.val * self.keysViewComp.height
			self.insidePos = self.keysTransformComp.worldTransform * self.insidePos
		except:
			pass
		return self.insidePos

	def SetNavRange(self):
		w = self.keysViewComp.width
		h = self.keysViewComp.height
		xfm = self.keysCamComp.worldTransform
		size = xfm * tdu.Vector(w, h, 1.0)
		x0 = xfm[0, 3] 
		y0 = xfm[1, 3]
		x1 = x0 + size.x
		y1 = y0 + size.y
		self.KeysViewHorzRange = (x0, x1)
		self.KeysViewVertRange = (y0, y1)
		self.setLabelsTy()

	def setLabelsTy(self):
		self.KeysViewVertRange
		# sampleIndex = np.round(
		# 	vMath.rangeValue(
		# 		self.channelLabelsPos[0],
		# 		0,
		# 		self.keysViewComp.width,
		# 		0,
		# 		self.KeyframeLookupChop.numSamples
		# 	)
		# )
		# # TODO use ...Chop.numpyArray() instead...?
		# for i, chopChan in enumerate(self.KeyframeLookupChop.chans()):	
		# 	self.channelLabelsPos[1][i] = chopChan[sampleIndex[i]]	

		# ty = vMath.rangeValue(
		# 	self.channelLabelsPos[1],
		# 	self.KeysViewVertRange[0],
		# 	self.KeysViewVertRange[1],
		# 	0,
		# 	self.keysViewComp.height
		# )
		# for i, par in enumerate(self.channelLabelTyPars):
		# 	par.val = ty[i]

	def updateKeysView(self, init=False):
		self.setLabelsTy()

	def toggleSelectedLockHandles(self):
		for chan in self.selectedChannels:
			chan.toggleSelectedLockHandles()

	def OnDropView(self, comp, info):
		for item in info['dragItems']:
			_type = type(item)
			if _type == Par:
				if (item.isNumber or item.isMenu 
								or item.isToggle 
								or item.isPulse):
					name = f"{item.owner.name}_{item.name}"
					if not self.Channels.get(name, False):
						self.AppendChannel(name, startVal=item.eval())
						outChop = self.AnimationChop.op('out')
						shortCutPath = item.owner.shortcutPath(outChop)
						expr = f"{shortCutPath}['{name}']"
						run("args[0].expr = args[1]", item, expr, delayFrames=1)
					else:
						print(
							f"Animation Comp already has channel named: {name}")
				else:
					print('Dropped non numeric par')
			elif _type == TD_CHOP_CHANNEL:
				name = f"{item.owner.name}_{item.name}"
				if not self.Channels.get(name, False):
					self.AppendChannel(name, startVal=item.eval())
				else:
					print(
						f"Animation Comp already has channel named: {name}")
			elif isinstance(item, CHOP):
				for chopChan in item.chans():
					name = f"{item.name}_{chopChan.name}"
					if not self.Channels.get(name, False):
						self.AppendChannel(name, updateKeysView=False, 
										startVal=chopChan.eval())
					else:
						print(
						f"Animation Comp already has channel named: {name}")
				run("args[0](init=True)", self.updateKeysView,
					delayFrames=item.numChans + 1)
			elif isinstance(item, DAT):
				mode = ui.messageBox('Create Channels From DAT Mode', 
					'Create Channels', 
					buttons=['Cancel', 'From First Row', 'From First Column']
				)
				if mode > 0:
					cells = item.row(0) if mode == 1 else item.col(0)
					for i, cell in enumerate(cells):
						if cell.val.isidentifier():
							name = f"{cell.val}"
							if not self.Channels.get(name, False):
								self.AppendChannel(name, updateKeysView=False)						
							else:
								print(
								f"Animation Comp already has channel named: {name}")
						else:
							print(
								f"{cell.val} is not a valid channel name")									
					run("args[0](init=True)", self.updateKeysView,
						delayFrames=len(cells) + 1)						
			elif isinstance(item, animationCOMP):
				if 'KEYFRAMER_ANIM_COMP' not in item.tags:
					m = (f"The animationCOMP: {item.path} is not 'Keyframer' "
						f"configured would you like to convert it?")
					confirm = ui.messageBox('Convert', m, 
											buttons=['Cancel', 'convert'])
					if confirm == 1:
						comp = self.convertToKeyframerAnimComp(item)
						if comp is not None:
							self.AnimationChop = comp
						else:
							print(f"Unable to convert: {item.path}")	
				else:	
					if item == self.defaultAnimationChop:
						print('Dropped in Keyframer, choose another location.')
					else:
						self.AnimationChop = item
			else:
				print('Dropped invalid object.')

	def OnNewAnimDragStart(self, info):
		return [self.defaultAnimationChop]

	def OnNewAnimDragEnd(self, info):
		if info['accepted']:
			results = info.get('dropResults')
			if results is not None:
				createdOPs = results.get('createdOPs', [])
				if len(createdOPs) > 0:
					animComp = createdOPs[0]
					name = self.newAnimNameComp.par.Value.eval()
					if name == '':
						name = 'animation1'
					if animComp.parent().op(name) is None:
						animComp.name = name
					else:
						print(f"{name} animation comp already exists!")
					self.animCompComp.UpdateView(animComp)

	def OnAppendChannels(self):
		# prevState = self.getAnimationCompState()		
		names = self.newChannelNamesComp.par.Value.eval().split(' ')
		for i, name in enumerate(names):
			self.AppendChannel(name, updateKeysView=False)
		# run("args[0](init=True)", self.updateKeysView, delayFrames=len(names) + 1)		
		# curState = self.getAnimationCompState()
		# ui.undo.startBlock(self.undoStateAnimCompName)
		# ui.undo.addCallback(self.undoDeleteAppendChannels, 
		# 					[prevState, curState])
		# ui.undo.endBlock()


	def OpenContextMenu(self, fromComp, *args):
		info = self.contextMenuLookup[fromComp.name]
		self.contextMenuComp.OpenMenu(fromComp,
			info['menuItems'],
			*args
		)

	def KeyframeControlsUpdateView(self, updateFunction=True):
		first_kf_sel_index = -1
		keyframe = None
		for i in range(self.keyframes_viewChop.numSamples):
			if self.keyframes_viewChop['selected'][i]:
				first_kf_sel_index = i
				break

		if first_kf_sel_index >= 0:
			channel = self.AnimationChop.channels[int(
				self.keyframes_viewChop['channel_index'][first_kf_sel_index])]
			keyframe = channel.keyframe(int(
				self.keyframes_viewChop['keyframe_index'][first_kf_sel_index]))
		else:
			for i in range(self.segments_viewChop.numSamples):
				if self.segments_viewChop['selected_start_handles'][i]:
					channel_index = int(
						self.segments_viewChop['channel_index'][i])
					segment_index = int(self.segments_viewChop['segment_index'][i])
					keyframe = self.AnimationChop.channels[channel_index].keyframe(
						segment_index)

				elif self.segments_viewChop['selected_end_handles'][i]:
					channel_index = int(
						self.segments_viewChop['channel_index'][i])
					segment_index = int(self.segments_viewChop['segment_index'][i]) + 1
					keyframe = self.AnimationChop.channels[channel_index].keyframe(
						segment_index)	
					break


		if keyframe is not None:
			self.keyframeControlsComp.UpdateViewKey(
				keyframe.time, 
				keyframe.value,
			)
			# if updateFunction:
			self.keyframeControlsComp.UpdateViewFunction(
			keyframe.function
			)
			self.keyframeControlsComp.UpdateViewHandleMode(
				keyframe.handle_mode
			)
						
		# if self.inHandlesSelected:
			self.keyframeControlsComp.UpdateViewInHandle(
				keyframe.in_handle.time, 
				keyframe.in_handle.value,
			)
		# if self.outHandlesSelected:
			self.keyframeControlsComp.UpdateViewOutHandle(
				keyframe.out_handle.time, 
				keyframe.out_handle.value,
			)		
	
	def GetKeyHandlesActive(self):
		self.keysSelected = False
		self.inHandlesSelected = False
		self.outHandlesSelected = False
		self.lastSelectedKey = None
		self.lastSelectedInHandle = None
		self.lastSelectedOutHandle = None
		for chan in self.Channels.values():
			if len(chan.selectedKeys) > 0:
				self.keysSelected = True
				if self.updateViewKeySegment is None:
					self.updateViewKeySegment = chan.selectedKeys[0]
			if len(chan.selectedHandles) > 0:	
				for selectedHandle in reversed(chan.selectedHandles):
					if selectedHandle % 2 == 0:
						self.inHandlesSelected = True
						if self.updateViewInHandleSegment is None:
							self.updateViewInHandleSegment = selectedHandle
					else:
						self.outHandlesSelected = True
						if self.updateViewOutHandleSegment is None:
							self.updateViewOutHandleSegment = selectedHandle						
		return self.keysSelected, self.inHandlesSelected, self.outHandlesSelected
	
	def OnChannelListSetValue(self, element, value):
		self.curves_viewChop.set_channel_display(value[0], value[2])
		self.updateKeysView()

	def SetFrameSelectedKeyframes(self, frame):
		for chan in self.Channels.values():
			for _id in chan.selectedKeys:
				if _id < chan.numSegments:
					segment = chan.segments[_id]
					value = segment.y0
					segment.setInXY(frame, value)
					segment.startSetKeyIn.x = segment.x0
				else:
					segment = chan.segments[_id - 1]
					value = segment.y1
					segment.setOutXY(frame, value)
					segment.startSetKeyOut.x= segment.x1	
		self.setLabelsTy()

	def SetValueSelectedKeyframes(self, value):
		for chan in self.Channels.values():
			for _id in chan.selectedKeys:
				if _id < chan.numSegments:
					segment = chan.segments[_id]
					frame = segment.x0
					segment.setInXY(frame, value)
					segment.startSetKeyIn.y = segment.y0
				else:
					segment = chan.segments[_id - 1]
					frame = segment.x1
					segment.setOutXY(frame, value)
					segment.startSetKeyOut.y = segment.y1				
		self.setLabelsTy()

	def SetInslopeSelectedKeyframes(self, slope):
		for chan in self.Channels.values():
			for _id in chan.selectedHandles:
				if _id % 2 == 0:
					segmentIndex = math.floor(_id / 2.0)
					segment = chan.segments[segmentIndex]
					segment.setInSlopeAccel(slope=slope)
			for _id in chan.selectedKeys:
				segment = chan.segments[_id]
				if segment.hasHandles:
					segment.setInSlopeAccel(slope=slope)					
		self.setLabelsTy()

	def SetInaccelSelectedKeyframes(self, accel):
		accel = max(0, accel)
		for chan in self.Channels.values():
			for _id in chan.selectedHandles:
				if _id % 2 == 0:
					segmentIndex = math.floor(_id / 2.0)
					segment = chan.segments[segmentIndex]
					segment.setInSlopeAccel(accel=accel)
			for _id in chan.selectedKeys:
				segment = chan.segments[_id]
				if segment.hasHandles:
					segment.setInSlopeAccel(accel=accel)	
		self.setLabelsTy()

	def SetOutslopeSelectedKeyframes(self, slope):
		for chan in self.Channels.values():
			for _id in chan.selectedHandles:
				if _id % 2 == 1:
					segmentIndex = math.floor(_id / 2.0)
					segment = chan.segments[segmentIndex]
					segment.setOutSlopeAccel(slope=slope)
			for _id in chan.selectedKeys:
				if _id != 0:
					segment = chan.segments[_id - 1]
					if segment.hasHandles:
						segment.setOutSlopeAccel(slope=slope)
		self.setLabelsTy()

	def SetOutAccelSelectedKeyframes(self, accel):
		accel = max(0, accel)
		for chan in self.Channels.values():
			for _id in chan.selectedHandles:
				if _id % 2 == 1:
					segmentIndex = math.floor(_id / 2.0)
					segment = chan.segments[segmentIndex]
					segment.setOutSlopeAccel(accel=accel)
			for _id in chan.selectedKeys:
				if _id != 0:
					segment = chan.segments[_id - 1]
					if segment.hasHandles:
						segment.setOutSlopeAccel(accel=accel)					
		self.setLabelsTy()

	def SetFunctionSelectedKeyframes(self, i):
		# TODO needs it's own undo function
		self.SetPrevStateChannels()			
		viewKeySegment = (self.updateViewKeySegment.owner.name, 
						self.updateViewKeySegment.index)
		selectedKeys = {}
		for name, chan in self.Channels.items():
			selectedKeys[name] = []
			for _id in chan.selectedKeys:
				selectedKeys[name].append(_id)
		function = self.segmentFuncNames[i]
		editedSegments = []
		for chan in self.Channels.values():		
			for _id in chan.selectedKeys:
				if _id < chan.numSegments:
					segment = chan.segments[_id]
					editedSegments.append((chan.name, _id))
					if function not in chan.altFuncs.keys():
						segment.keyInRow[5].val = function
						segment.keyInRow[9].val = ''
						if segment.isLastSegment:
							segment.keyOutRow[5].val = function
							segment.keyOutRow[9].val = ''							
					else:
						segment.keyInRow[5].val = chan.altFuncs[function]
						segment.keyInRow[9].val = function
						if segment.isLastSegment:
							segment.keyOutRow[5].val = chan.altFuncs[function]
							segment.keyOutRow[9].val = function					
		self.SetChannels()
		for chanName, segmentIndex in editedSegments:
			chan = self.Channels[chanName]
			if segmentIndex < chan.numSegments:
				segment = chan.segments[segmentIndex]
				if not segment.isLastSegment:
					nextSegment = chan.segments[segmentIndex + 1]
					if segment.segmentType != nextSegment.segmentType:
						segment.lockHandles = False
				if segment.index > 0:
					prevSegment = chan.segments[segmentIndex - 1]
					if segment.segmentType != prevSegment.segmentType:
						prevSegment.lockHandles = False	
		self.curves_viewChop.unselect_all()
		self.setLabelsTy()
		for chanName, _ids in selectedKeys.items():
			chan = self.Channels[chanName]
			for _id in _ids:
				chan.selectKey(_id, 1)
				if chanName == viewKeySegment[0] and _id == viewKeySegment[1]:
					self.updateViewKeySegment = chan.segments[_id]
					self.GetKeyHandlesActive()
					self.KeyframeControlsUpdateView(updateFunction=True)
		self.SetCurStateChannels()

	def ReorderChannels(self, start, end):
		self.SetPrevStateChannels()		
		start += 1
		end += 1
		rowVals = [cell.val for cell in self.channelsDat.row(start)]
		if start < end:
			self.channelsDat.appendRow(rowVals, end)
			self.channelsDat.deleteRow(start)
		elif start > end:
			self.channelsDat.deleteRow(start)
			self.channelsDat.insertRow(rowVals, end)	
		self.curves_viewChop.unselect_all()
		self.SetChannels()
		self.updateKeysView(init=True)
		self.SetCurStateChannels(force=True)	

	def convertToKeyframerAnimComp(self, comp):
		print(F"Conversion of Animation Comp not implemented yet: {comp.path}")
