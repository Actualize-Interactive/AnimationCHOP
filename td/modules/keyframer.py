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
		self.channels_replicatorComp = self.keysViewComp.op('channels/replicator1')

		self.MasterKeyboardChop = self.viewComp.op('MasterKeyboard')
		self.keysRenderPickComp = self.viewComp.op('timeGraph')
		self.channelControlsComp = self.viewComp.op('channelControls')
		self.channelListComp = self.channelControlsComp.op('channelList')
		self.editControlsComp = self.viewComp.op('editControls')
		self.keyframeControlsComp = self.editControlsComp.op('keyframeControls')
		self.contextMenuComp = self.viewComp.op('ContextMenu')	
		self.globalControlsComp = self.viewComp.op('globalControls')
		self.display_keys_active = self.globalControlsComp.op('displayKeysHandles').par.Valuekeys
		self.display_handles_active = self.globalControlsComp.op('displayKeysHandles').par.Valuehandles
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

		self.SetSelectedFuncs = {
			'offset_time': self.SetOffsetTimeSelectedKeyframes,
			'offset_value': self.SetOffsetValueSelectedKeyframes,
			'time': self.SetTimeSelectedKeyframes,
			'value': self.SetValueSelectedKeyframes,
			'in_handle_time': self.SetInHandleTimeSelectedKeyframes,
			'in_handle_value': self.SetInHandleValueSelectedKeyframes,
			'out_handle_time': self.SetOutHandleTimeSelectedKeyframes,
			'out_handle_value': self.SetOutHandleValueSelectedKeyframes,
			'function': self.SetFunctionSelectedKeyframes,
			'handle_mode': self.SetHandleModeSelectedKeyframes,

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
	def AnimationChop(self): 
		if self.ownerComp.par.Animationchop.eval() is not None:
			return self.ownerComp.par.Animationchop.eval()
		else:
			return self.default_animationChop
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
	def prevStateChannels(self):
		return self.ownerComp.storage.get('prevStateChannels')
	@prevStateChannels.setter
	def prevStateChannels(self, value):
		self.ownerComp.storage['prevStateChannels'] = value

	@property
	def ChannelNames(self):
		return tdu.Dependency(list(self.AnimationChop.channel_names))

	@property
	def NumChannels(self):
		return tdu.Dependency(self.AnimationChop.num_channels)

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
		self.begin_set_item_offset_pos = tdu.Position()
		self.set_item_offset = tdu.Position()
		self.set_item_info = None
		self.insertPos = tdu.Position()
		self.insidePos = tdu.Position()

		try:
			if newAnimComp:
				channelComps = self.ChannelsComp.findChildren()
				for channelComp in channelComps:
					if channelComp:
						channelComp.destroy()				
			self.refreshChannelList()
			self.curves_viewChop.unselect_all()
			self.AnimationChop.cook(force=True, recurse=True)
			if newAnimComp:
				self.keysNavHome()
				# self.updateKeysView(init=True)							
		except Exception as e:
			# on startup the AnimationChop might not be set yet
			# traceback.print_exc()
			pass

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

	def refreshChannelList(self):
		if self.channels_viewChop['display'] is not None:
			channels_display = self.channels_viewChop['display'].vals
		else:
			channels_display = [True] * self.AnimationChop.num_channels
		
		channel_names = self.AnimationChop.channel_names
		self.channelListComp.Refresh(channels_display, channel_names)
		self.channels_replicatorComp.cook(force=True)

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

			if self.keysPanel.lselect:
				if (event.select and event.pickOp is not None 
								and event.pickOp == self.startPickOP
								and not self.marqueeSelecting):			
					if not self.kd and not self.kf:
						self.offset_selected_on_pick(event)
					else:
						self.scaleKeys(event)
				elif event.select:
					self.marqueeSelect(event)
			elif self.keysPanel.mselect:
				self.keysNav(event)

			if event.selectEnd:
				self.keysNavEnd(event)	
				self.onPickEnd(event)

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

	def AppendChannel(self, name, startVal=None):
		if name == '' or name == 'name' or self.AnimationChop.has_channel(name):
			print(f"'{name}' is an invalid channel name.")
			return False
		else:
			channel = self.AnimationChop.create_channel(name)
			if startVal is None:
				startVal = 0.0
			channel.create_keyframe(0.0, startVal)
			channel.create_keyframe(5.0, startVal)
		
		self.refreshChannelList()

	def DeleteChannel(self, name):
		if self.AnimationChop.has_channel(name):
			self.AnimationChop.remove_channel(name)
		self.refreshChannelList()

	def DeleteAllChannels(self):
		self.AnimationChop.clear()
		self.refreshChannelList()

	def DeleteChannelConfirm(self, chanName):
		confirm = ui.messageBox('Delete Channel',
			f"Are you sure you would like to delete: {chanName}?",
			buttons = ['Cancel', 'Delete'])
		if confirm == 1:
			self.curves_viewChop.cache_state()
			self.DeleteChannel(chanName)
			self.set_undo('Keyframer: Delete Channel')

	def DeleteChannelsConfirm(self, chanName):
		confirm = ui.messageBox('Delete Channel',
			f"Are you sure you would like to delete all displayed channels?",
			buttons = ['Cancel', 'Delete'])
		if confirm == 1:
			self.curves_viewChop.cache_state()
			channels_to_delete = []
			for i, val in enumerate(self.channels_viewChop['display'].vals):
				if val:
					channel = self.AnimationChop.channels[i]
					channels_to_delete.append(channel.name)
					
			for chan_name in channels_to_delete:
				self.DeleteChannel(chan_name)

			self.set_undo('Keyframer: Delete Channels')

	def EditChannelName(self, chanName):
		idx = None
		for i, name in enumerate(self.AnimationChop.channel_names):
			if name == chanName:
				idx = i
				break
		if idx is not None:
			self.channelListComp.StartEditCell(idx)

	def RenameChannel(self, channel_index, new_name):
		if new_name in self.AnimationChop.channel_names:
			ui.messageBox('Warning', 
				f"{new_name} already exists! Please use another name.",
				buttons=['Close'])
			return
		self.curves_viewChop.cache_state()
		self.AnimationChop.channels[channel_index].name = new_name
		self.set_undo(f'Keyframer: Rename Channel {self.AnimationChop.channels[channel_index].name} to {new_name}')
		self.refreshChannelList()

	def ReorderChannels(self, start, end):
		print(f"Reorder Channels not implemented")

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
		self.KeyframeControlsUpdateView()	

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

		self.KeyframeControlsUpdateView()	

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
		self.copiedKeys = []
		for kv in self.keyframes_viewChop.selected_keyframes():
			c = kv['channel_index']
			k = kv['keyframe_index']
			kf = self.AnimationChop.channels[c][k]
			state = kf.state
			state['channel_index'] = c
			self.copiedKeys.append(state)

	def cutKeys(self):
		self.copyKeys()
		self.DeleteSelectedKeys()

	def pasteKeys(self):
		if self.keysPanel.inside:
			self.insertPos.x = self.keysPanel.insideu * self.keysViewComp.width
			self.insertPos.y = self.keysPanel.insidev * self.keysViewComp.height
			self.insertPos = (self.keysTransformComp.worldTransform 
							* self.insertPos)
			
			# find the minimum time value
			minTime = min(kf_state['position']['time'] for kf_state in self.copiedKeys)
			offset = self.insertPos.x - minTime

			for kf_state in self.copiedKeys:
				c = kf_state['channel_index']
				if c < len(self.AnimationChop.channels):
					chan = self.AnimationChop.channels[c]
					# kf_state.pop('channel_index', None)
					kf_state['position']['time'] += offset
					chan.create_keyframe_from_state(kf_state)

		self.unSelectAll()
			
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

		self.update_active_controls()

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

	def offset_selected_on_pick(self, event):
		geo = event.pickOp.parent()
		i = geo.par.Geotype.menuIndex
		indices = event.custom['indices']

		if i < 3:
			self.set_item_offset = self.get_item_offset(event)
			
			if self.kshift:
				self.set_item_offset.y = 0
			elif self.kctrl:
				self.set_item_offset.x = 0
			if i == 0:
				self.curves_viewChop.offset_selected_keyframes(self.set_item_offset.x, self.set_item_offset.y)
				self.set_item_info = [indices[0], indices[1]] # channel_index, keyframe_index
			else:
				self.curves_viewChop.offset_selected_end_handles(self.set_item_offset.x, self.set_item_offset.y)
				self.curves_viewChop.offset_selected_start_handles(self.set_item_offset.x, self.set_item_offset.y)
				if i == 1:
					self.set_item_info = [indices[0], indices[1] + 1]
				else:
					self.set_item_info = [indices[0], indices[1]]

			# self.keyframeControlsComp.Active(True)

		self.setLabelsTy()
		self.KeyframeControlsUpdateView()

	def get_item_offset(self, event):
		new_position = tdu.Position(
			event.u * self.keysViewComp.width,
			event.v * self.keysViewComp.height, 0)
		new_position = self.keysTransformComp.worldTransform * new_position
		return new_position - self.begin_set_item_offset_pos

	def onPickSelectStart(self, event):
		self.cache_state()
		self.set_item_offset.x = 0
		self.set_item_offset.y = 0
		self.begin_set_item_offset_pos.x = event.u * self.keysViewComp.width
		self.begin_set_item_offset_pos.y = event.v * self.keysViewComp.height
		self.begin_set_item_offset_pos = self.keysTransformComp.worldTransform * self.begin_set_item_offset_pos

		self.pickStartVals = {}
		self.pickStartVals['uv'] = (event.u, event.v)
		self.pickStartVals['shift'] = self.keysViewComp.panel.shift.val
		self.pickStartVals['ctrl'] = self.keysViewComp.panel.ctrl.val
		self.pickStartVals['alt'] = self.keysViewComp.panel.alt.val

	def onPickEnd(self, event):
		if self.switchMarqueeTop.par.index:	
			self.marqueeSelectEnd()
		else:
			self.set_undo()
			
		self.startSet = False
		self.startScale = False
		self.set_item_offset.x = 0
		self.set_item_offset.y = 0
		
		if self.curves_viewChop.selected_keyframes() == [] and \
				self.curves_viewChop.selected_start_handles() == [] and \
				self.curves_viewChop.selected_end_handles() == []:
			self.set_item_info = None


		
		self.update_active_controls()
		self.KeyframeControlsUpdateView()

	def OnControlsActionStart(self):
		self.cache_state()

	def OnControlsActionEnd(self):
		self.set_undo()
		self.KeyframeControlsUpdateView()

	def undo_callback(self, isUndo, info):
		if isUndo:
			# print("undo_callback")
			self.curves_viewChop.undo()
			self.refreshChannelList()
		else:
			# print("redo_callback")
			self.curves_viewChop.redo()
			self.refreshChannelList()

	def cache_state(self):
		# print("cache_state")
		self.curves_viewChop.reset_begin_set_values()
		self.curves_viewChop.cache_state()
		pass

	def set_undo(self, message='Animation Editor: Update Keyframes'):
		# print("set_undo")
		if self.curves_viewChop.set_undo():
			# print("set_undo: True")
			ui.undo.startBlock(message)
			ui.undo.addCallback(self.undo_callback)
			ui.undo.endBlock()

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
			mCoords = [[self.begin_set_item_offset_pos.x, self.selectPos.x], 
						[self.begin_set_item_offset_pos.y, self.selectPos.y]]
			mCoords[0].sort()
			mCoords[1].sort()

			selected_indices = [] # sample_index
			if self.display_keys_active.eval():
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
			if self.display_handles_active.eval():
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

		self.keysRenderPickDat.par.strategy = 'holdfirst'
		self.switchMarqueeTop.par.index = 0	

	def OnKeyboardIn(self, key, character,
					alt, lAlt, rAlt, ctrl, lCtrl, rCtrl,
					shift, lShift, rShift, state, time, cmd, lCmd, rCmd):
		# TODO create lookup... 
		if state:
			
			if key == 'a' and ctrl:
				self.cache_state()	
				self.SelectAllKeys()
				self.set_undo()		
			elif key == 'h':
				# self.cache_state()	
				self.keysNavHome()
				# self.set_undo()		
			elif key == 't':
				self.cache_state()	
				self.toggleSelectedLockHandles()
				self.set_undo()		
			elif key == 'c' and ctrl:
				self.cache_state()	
				self.copyKeys()
				self.set_undo()		
			elif key == 'x' and ctrl:
				self.cache_state()	
				self.cutKeys()
				self.set_undo()		
			elif key == 'v' and ctrl:
				self.cache_state()	
				self.pasteKeys()
				self.set_undo()
			elif key == 'delete':
				self.cache_state()	
				self.DeleteSelectedKeys()
				self.set_undo()			
			elif key == 'backspace':
				self.cache_state()	
				self.DeleteSelectedKeys()
				self.set_undo()		
			elif key == 'tab':
				self.cache_state()	
				self.SelectAdjacentItem(-1 if shift else 1)
				self.set_undo()		
			elif key in ('right', 'left', 'up', 'down'):
				self.cache_state()	
				scale = 1.0
				if self.kshift:
					scale = 2.0
				elif self.kctrl:
					scale = 4.0
				elif self.kalt:
					scale = 8.0
				nudge = self.nudgeDir[key] * scale
				self.NudgeItem(nudge)	
				self.set_undo()		
				
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

	def OnAnimationChopChanged(self):
		self.refreshChannelList()
		self.update_active_controls()
		self.KeyframeControlsUpdateView()

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
						self.AppendChannel(name, startVal=chopChan.eval())
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
								self.AppendChannel(name)						
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
		self.append_channels(names)

	def append_channels(self, names):
		if len(names) == 0:
			return
		self.curves_viewChop.cache_state()
		for i, name in enumerate(names):
			self.AppendChannel(name)
		
		self.set_undo(f'Keyframer: Append Channels: {", ".join(names)}')

	def OpenContextMenu(self, fromComp, *args):
		info = self.contextMenuLookup[fromComp.name]
		self.contextMenuComp.OpenMenu(fromComp,
			info['menuItems'],
			*args
		)

	def update_active_controls(self):
		skv = self.curves_viewChop.selected_keyframes()
		len_skv = len(skv)
		sehv = self.curves_viewChop.selected_end_handles()
		len_sehv = len(sehv)
		sshv = self.curves_viewChop.selected_start_handles()
		len_sshv = len(sshv)

		nothing_selected = True

		if len_skv > 0:
			self.set_item_info = [skv[0]['channel_index'], skv[0]['keyframe_index']]
			self.keyframeControlsComp.ActiveOffset(True)
			self.keyframeControlsComp.ActiveKey(True)
			nothing_selected = False
		else:
			self.keyframeControlsComp.ActiveKey(False)
			
		if len_sshv > 0:
			self.set_item_info = [sshv[0]['channel_index'], sshv[0]['keyframe_index']]
			self.keyframeControlsComp.ActiveOffset(True)
			self.keyframeControlsComp.ActiveOutHandle(True)
			nothing_selected = False
		else:
			self.keyframeControlsComp.ActiveOutHandle(False)

		if len_sehv > 0:
			self.set_item_info = [sehv[0]['channel_index'], sehv[0]['keyframe_index'] + 1]
			self.keyframeControlsComp.ActiveOffset(True)
			self.keyframeControlsComp.ActiveInHandle(True)
			nothing_selected = False
		else: 
			self.keyframeControlsComp.ActiveInHandle(False)

		if nothing_selected:
			self.set_item_info = None
			self.keyframeControlsComp.Active(False)	

	def KeyframeControlsUpdateView(self, update_offset=True):
		if self.set_item_info is not None:
			keyframe = self.AnimationChop.channels[self.set_item_info[0]][self.set_item_info[1]]
			self.keyframeControlsComp.UpdateViewKey(
				keyframe.time, 
				keyframe.value,
			)
			self.keyframeControlsComp.UpdateViewFunction(keyframe.function, keyframe.handle_mode)
			self.keyframeControlsComp.UpdateViewOutHandle(
				keyframe.out_handle.time,
				keyframe.out_handle.value,
			)
			self.keyframeControlsComp.UpdateViewInHandle(
				keyframe.in_handle.time,
				keyframe.in_handle.value,
			)
			if update_offset:
				self.keyframeControlsComp.UpdateViewOffset(self.set_item_offset.x, 
													self.set_item_offset.y)

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
		# print(f"OnChannelListSetValue: {element}, {value}")
		self.curves_viewChop.set_channel_display(value[0], value[2])
		self.updateKeysView()

	def SetOffsetTimeSelectedKeyframes(self, offset):
		if len(self.curves_viewChop.selected_keyframes()) > 0:
			self.curves_viewChop.offset_selected_keyframes_by_time(offset)
		else:
			self.curves_viewChop.offset_selected_end_handles_by_time(offset)
			self.curves_viewChop.offset_selected_start_handles_by_time(offset)
		self.KeyframeControlsUpdateView(update_offset=False)
		self.setLabelsTy()
	
	def SetOffsetValueSelectedKeyframes(self, offset):
		if len(self.curves_viewChop.selected_keyframes()) > 0:
			self.curves_viewChop.offset_selected_keyframes_by_value(offset)
		else:
			self.curves_viewChop.offset_selected_end_handles_by_value(offset)
			self.curves_viewChop.offset_selected_start_handles_by_value(offset)
		self.KeyframeControlsUpdateView(update_offset=False)
		self.setLabelsTy()

	def SetTimeSelectedKeyframes(self, time):
		for kv in self.keyframes_viewChop.selected_keyframes():
			c = kv['channel_index']
			k = kv['keyframe_index']
			self.AnimationChop.channels[c].set_keyframe_time(k, time)
		self.setLabelsTy()

	def SetValueSelectedKeyframes(self, value):
		for kv in self.keyframes_viewChop.selected_keyframes():
			c = kv['channel_index']
			k = kv['keyframe_index']
			self.AnimationChop.channels[c].set_keyframe_value(k, value)				
		self.setLabelsTy()

	def SetInHandleTimeSelectedKeyframes(self, time):
		for sv in self.segments_viewChop.selected_end_handles():
			c = sv['channel_index']
			k = sv['keyframe_index'] + 1
			kf = self.AnimationChop.channels[c][k]
			point = self.AnimationChop.Point(time, kf.in_handle.value)
			self.AnimationChop.channels[c].set_keyframe_in_handle(k, point)
		self.setLabelsTy()

	def SetInHandleValueSelectedKeyframes(self, value):
		for sv in self.segments_viewChop.selected_end_handles():
			c = sv['channel_index']
			k = sv['keyframe_index'] + 1
			kf = self.AnimationChop.channels[c][k]
			point = self.AnimationChop.Point(kf.in_handle.time, value)
			self.AnimationChop.channels[c].set_keyframe_in_handle(k, point)
		self.setLabelsTy()

	def SetOutHandleTimeSelectedKeyframes(self, time):
		for sv in self.segments_viewChop.selected_start_handles():
			c = sv['channel_index']
			k = sv['keyframe_index']
			kf = self.AnimationChop.channels[c][k]
			point = self.AnimationChop.Point(time, kf.out_handle.value)
			self.AnimationChop.channels[c].set_keyframe_out_handle(k, point)
		self.setLabelsTy()

	def SetOutHandleValueSelectedKeyframes(self, value):
		for sv in self.segments_viewChop.selected_start_handles():
			c = sv['channel_index']
			k = sv['keyframe_index']
			kf = self.AnimationChop.channels[c][k]
			point = self.AnimationChop.Point(kf.out_handle.time, value)
			self.AnimationChop.channels[c].set_keyframe_out_handle(k, point)
		self.setLabelsTy()

	def SetFunctionSelectedKeyframes(self, i):
		for kv in self.keyframes_viewChop.selected_keyframes():
			c = kv['channel_index']
			k = kv['keyframe_index']
			self.AnimationChop.channels[c].set_keyframe_function(k, i)
		self.setLabelsTy()
		
	def SetHandleModeSelectedKeyframes(self, i):
		for kv in self.keyframes_viewChop.selected_keyframes():
			c = kv['channel_index']
			k = kv['keyframe_index']
			self.AnimationChop.channels[c].set_keyframe_handle_mode(k, i)
		pass

	def convertToKeyframerAnimComp(self, comp):
		print(F"Conversion of Animation Comp not implemented")
