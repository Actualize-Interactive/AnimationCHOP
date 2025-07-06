import re

pop_dialog = op.TDResources.PopDialog

class AnimCurvesExt:

	def __init__(self, ownerComp):
		# The component to which this extension is attached
		self.ownerComp = ownerComp
		self.anim = ownerComp.op('anim')

	@property
	def ChannelNames(self):
		return tdu.ParMenu(self.anim.channel_names)

	@property
	def CurrentChannel(self):
		if self.anim.num_channels > 0:
			menuIndex = self.ownerComp.par.Selectchannel.menuIndex
			return self.anim.channels[menuIndex]
		else:
			return None

		

	def updatePars(self):
		chan = self.CurrentChannel
		# self.ownerComp.par.Startindex = chan.Startindex
		# self.ownerComp.par.Numcycles = chan.Numcycles
		# self.ownerComp.par.Cyclestartoffset = chan.Cyclestartoffset
		# self.ownerComp.par.Cyclevalueoffset = chan.Cyclevalueoffset
		# self.ownerComp.par.Cyclevalmode = chan.Cyclevalmode
		# self.ownerComp.par.Inamplitude = chan.Inamplitude
		# self.ownerComp.par.Inoffset = chan.Inoffset		
		# self.ownerComp.par.Inlength = chan.Inlength
		# self.ownerComp.par.Instartslope = chan.Instartslope
		# self.ownerComp.par.Inendslope = chan.Inendslope

		# if chan.Inlength > 0.0:
		# 	self.ownerComp.par.Instartmag = chan.Instartmag / chan.Inlength
		# 	self.ownerComp.par.Inendmag = chan.Inendmag / chan.Inlength
		# else:
		# 	self.ownerComp.par.Instartmag = 0.0
		# 	self.ownerComp.par.Inendmag = 0.0

		# self.ownerComp.par.Holdlength = chan.Holdlength
		# self.ownerComp.par.Outamplitude = chan.Outamplitude
		# self.ownerComp.par.Outoffset = chan.Outoffset				
		# self.ownerComp.par.Outlength = chan.Outlength
		# self.ownerComp.par.Outstartslope = chan.Outstartslope
		# self.ownerComp.par.Outendslope = chan.Outendslope

		# if chan.Outlength > 0.0:
		# 	self.ownerComp.par.Outstartmag = chan.Outstartmag / chan.Outlength
		# 	self.ownerComp.par.Outendmag = chan.Outendmag / chan.Outlength
		# else:
		# 	self.ownerComp.par.Outstartmag = 0.0
		# 	self.ownerComp.par.Outendmag = 0.0
		self.ownerComp.cook(force=True)
	
	def Selectchannel(self, par):
		self.updatePars()

	def Createchannel(self):
		print('Create channel')
		def dialogChoice(info):
			print(info)
			if info['buttonNum'] == 1:
				name = self.sanitize_channel_name(info['enteredText'])
				if name is not None:
					print('Creating channel with name:', name)

					self.anim.create_channel(name)
					
			# new_channel_name = details.get('text', 'NewChannel')
			# self.anim_chop.create_channel(new_channel_name)
			# self.ownerComp.par.Selectchannel.menu = self.ChannelNames
			# self.ownerComp.par.Selectchannel.menuIndex = len(self.Channels) - 1
			# self.updatePars()
			self.updatePars()


		pop_dialog.OpenDefault(
			text='Enter new channel name',
			title='Create Channel',
			buttons=['create', 'cancel'],
			callback=dialogChoice,
			details=None,
			textEntry=True,
			escButton=2,
			enterButton=1,
			escOnClickAway=True)
		pass
	
	def Duplicatechannel(self):
		if self.CurrentChannel is not None:
			# if the end of the channel name is a number, increment it
			name = self.CurrentChannel.name
			match = re.search(r'_(\d+)$', name)
			if match:
				base_name = name[:match.start()]
				num = int(match.group(1)) + 1
				new_name = f"{base_name}_{num}"
			else:
				new_name = f"{name}_1"

			new_name = self.sanitize_channel_name(new_name)
			if new_name is not None:
				# self.anim.duplicate_channel(self.CurrentChannel.name, new_name)
				# self.ownerComp.par.Selectchannel.menu = self.ChannelNames
				# self.ownerComp.par.Selectchannel.menuIndex = len(self.ChannelNames) - 1
				self.updatePars()

	def Renamechannel(self):
		pass

	def Deletechannel(self):
		if self.anim.num_channels > 0:
			confirm = ui.messageBox('Delete Channel', 
						'Are you sure you want to delete {}?'.format(self.CurrentChannel.name),
						buttons=['Cancel', 'Delete Channel'])		
			if confirm == 1:
				self.anim.remove_channel(self.CurrentChannel.name)
				self.updatePars()

	def Deleteallchannels(self):
		if self.anim.num_channels > 0:
			confirm = ui.messageBox('Delete All Channels', 
						'Are you sure you want to delete all channels?',
						buttons=['Cancel', 'Delete All Channels'])	
			if confirm == 1:
				self.anim.clear()
				self.updatePars()

	def sanitize_channel_name(self, name):
		# Remove any characters that are not alphanumeric or underscores
		name = re.sub(r'[^a-zA-Z0-9_]', '', name)

		if not name:
			name = f"curve_{self.anim.num_channels + 1}"

		if name in self.anim.channel_names:
			pop_dialog.OpenDefault(
				text='Channel name "{}" already exists. Please choose a different name.'.format(name),
				title='Duplicate Channel Name',
				buttons=['OK'],
				escButton=0,
				enterButton=0,
				escOnClickAway=True)
			return None
		
		return name
