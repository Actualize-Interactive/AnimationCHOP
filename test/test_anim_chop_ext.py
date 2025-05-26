from TDFunctions import parMenu
popDialog = op.TDResources.PopDialog


class TestAnimChopExt:
	"""
	testanimchopext description
	"""
	def __init__(self, ownerComp):
		# The component to which this extension is attached
		self.ownerComp = ownerComp
		self.animChop = ownerComp.op('Animationchop')

	@property
	def SelectChannelMenuData(self):
		chan_names = self.animChop.get_channel_names()
		return parMenu(chan_names, chan_names)
	
	@property
	def SelectedChannel(self):
		current_channel_name = self.ownerComp.par.Selectchannel.eval()
		if current_channel_name is None or current_channel_name == '':
			return
		
		if current_channel_name not in self.animChop.get_channel_names():
			print(f"Channel '{current_channel_name}' does not exist.")
			return
		
		return self.animChop.get_channel(current_channel_name)
	
	def find_unique_channel_name(self, base_name):
		base_name = base_name.strip()
		if not base_name:
			return

		channel_names = self.animChop.get_channel_names()
		if base_name not in channel_names:
			return base_name

		i = 1
		while True:
			new_name = f"{base_name}{i}"
			if new_name not in channel_names:
				return new_name
			i += 1

	def Createchannel(self):
		def callback(info):
			# print('Createchannel callback', info)
			if info['button'] == 'Create':
				channel_name = info['enteredText']
				if channel_name is not None:
					unique_name = self.find_unique_channel_name(channel_name)
					self.animChop.create_channel(unique_name)
					self.ownerComp.cook(force=True)
					print(f'Channel "{unique_name}" created.')
				else:
					print('No channel name provided.')
			else:
				print('Channel creation cancelled.')

		popDialog.Open(text='Enter channel name:',
				  title='Create Channel',
				  callback=callback,
				  buttons=['Create', 'Cancel'],
				  textEntry='channel',
		)

	def Selectchannel(self, channel_name):
		selected_channel = self.SelectedChannel
		if selected_channel is None:
			return
		
	def Createkeyframe(self):
		selected_channel = self.SelectedChannel
		if selected_channel is None:
			print('No channel selected.')
			return
		
		selected_channel.create_keyframe()
		self.ownerComp.cook(force=True)
		print(f'Keyframe created for channel "{selected_channel.name}".')
		




