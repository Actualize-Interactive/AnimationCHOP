WIDGETS = iop.Widgets
KEYFRAMER = parent.Keyframer

class WidgetVecExt(WIDGETS.Widget):
	"""
	WidgetVecExt description
	"""
	def __init__(self, ownerComp):
		super().__init__(ownerComp)	

	def SetValue(self, element, value):
		KEYFRAMER.SetSelectedFuncs[element.name](value)

	def UpdateViews(self, elementsValues):
		for key,value in elementsValues.items():
			if key != 'function':
				self.elementDict[key].UpdateView(None, value)	
			else:
				self.elementDict[key].UpdateView(value)	
			# print(key, value)	

	def UpdateView(self, element, value):
		self.elementDict[element].UpdateView(None, value)

	def UpdateViewOffset(self, time, value):
		self.elementDict['offset_time'].UpdateView(None, time)
		self.elementDict['offset_value'].UpdateView(None, value)

	def UpdateViewKey(self, time, value):
		self.elementDict['time'].UpdateView(None, time)
		self.elementDict['value'].UpdateView(None, value)

	def UpdateViewInHandle(self, slope, accel):
		self.elementDict['in_handle_time'].UpdateView(None, slope)
		self.elementDict['in_handle_value'].UpdateView(None, accel)

	def UpdateViewOutHandle(self, slope, accel):
		self.elementDict['out_handle_time'].UpdateView(None, slope)
		self.elementDict['out_handle_value'].UpdateView(None, accel)

	def UpdateViewFunction(self, function, handle_mode):
		self.elementDict['function'].UpdateView(function)
		if function == 2:
			self.elementDict['handle_mode'].UpdateView(handle_mode)
		else:
			self.elementDict['handle_mode'].UpdateView(None)


	def UpdateViewHandleMode(self, value):
		self.elementDict['handle_mode'].UpdateView(value)

	def GetState(self):
		fullState = {key:e.GetState() for key,e in self.elementDict.items()
				if e in self.elementsUpdate}
		state = {}
		for key, val in fullState.items():
			if isinstance(val, dict):
				state[key] = val['Field']
			else:
				state[key] = val
		return state

	def ActiveElement(self, name, value):
		self.elementDict[name].Active(value)

	def ActiveOffset(self, active):
		self.elementDict['offset_time'].Active(active)
		self.elementDict['offset_value'].Active(active)

	def ActiveKey(self, active):
		self.elementDict['time'].Active(active)
		self.elementDict['value'].Active(active)	
		self.elementDict['function'].Active(active)		
		self.elementDict['handle_mode'].Active(active)	

	def ActiveFunction(self, active):
		self.elementDict['function'].Active(active)
		self.elementDict['handle_mode'].Active(active)

	def ActiveInHandle(self, active):
		self.elementDict['in_handle_time'].Active(active)
		self.elementDict['in_handle_value'].Active(active)

	def ActiveOutHandle(self, active):
		self.elementDict['out_handle_time'].Active(active)
		self.elementDict['out_handle_value'].Active(active)

	def ActiveAll(self, key, inHandle, outHandle):
		self.ActiveKey(key)
		self.ActiveInHandle(inHandle)
		self.ActiveOutHandle(outHandle)				