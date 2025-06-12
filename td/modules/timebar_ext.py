
class TimeGraph:
	# TODO implement better method - use Numpy and consolidate TimerBarExt
	def __init__(self, ownerComp):
		self.ownerComp = ownerComp
		self.keyframerComp = ownerComp.parent.Keyframer
		self.viewComp = ownerComp.parent()
		self.keysViewComp = self.viewComp.op('keysView')
		self.timerBarComp = ownerComp.op('timerBar')
		self.valueLabelsComp = self.keysViewComp.op('valueLabels')
		self.valueLabelComps = self.valueLabelsComp.findChildren(name='label*')
		self.valueLabelComps.sort(key=lambda x: x.name)
		self.valueLabelPars = [(c.op('text').par.text, c.par.y, c.par.display) 
								for c in self.valueLabelComps]
		self.numValueLabelComps = len(self.valueLabelComps)
		self.GetDim()
	
	@property
	def Animrangeend(self):
		return self.ownerComp.par.Animrangeend.eval()

	@property
	def Animrangestart(self):
		return self.ownerComp.par.Animrangestart.eval()

	@property
	def numValueLabels(self):
		return self.ownerComp.storage.get('numValueLabels', 0)
	@numValueLabels.setter
	def numValueLabels(self, value):
		self.ownerComp.storage['numValueLabels'] = value

	def GetDim(self):
		self.h = self.keysViewComp.width
		self.w = self.keysViewComp.height
		self.graphH = self.h # - 15
		self.graphW = self.w

	def SetView(self):
		# self.SetTime()
		# self.SetValue()
		# TODO implement graph in with geometry instancing in keysView/render
		# below is a temp workaround due to the transform being behind 1 frame
		run("args[0]()", self.setView, delayFrames=1)

	def setView(self):
		self.SetTime()
		self.SetValue()	

	def SetTime(self):
		animStart = self.keyframerComp.AnimationRangeStart	
		animEnd = self.keyframerComp.AnimationRangeEnd
		horzRange = self.keyframerComp.KeysViewHorzRange
		start = vMath.rangeValue(animStart, *horzRange, 
								0, self.keysViewComp.width)
		end = vMath.rangeValue(animEnd, *horzRange, 0, self.keysViewComp.width)
		length = abs(end - start)
		timeWidth = length / 4 * (60 / animEnd)
		timeWidthMin = self.ownerComp.par.Timewidthmin.eval()
		minRatio = timeWidthMin / timeWidth
		minRatio = math.pow(2, math.ceil(math.log(minRatio) / math.log(2)))

		self.ownerComp.par.Timeoffset = start
		self.ownerComp.par.Endbound = length
		self.ownerComp.par.Timewidth = timeWidth * minRatio

	def SetValue(self):
		vRangeMin = self.keyframerComp.KeysViewVertRange[0]
		vRangeMax = self.keyframerComp.KeysViewVertRange[1]
		valueOffset = vMath.rangeValue(0, vRangeMin, vRangeMax, 
										0, self.keysViewComp.height)
		vRange = vRangeMax - vRangeMin
		scaleFactor = self.ownerComp.par.Valuelinesscalefactor.eval()
		vRangeRound = self.ArbRound(vRange, scaleFactor, vRangeMin)
		vRange = 1.0 / vRange * self.keysViewComp.height * .5 * vRangeRound
		self.ownerComp.par.Valueoffset = valueOffset
		self.ownerComp.par.Valueheight = vRange

	def arbRound(self, x, prec=2, base=.05):
		return round(base * round(float(x) / base), prec)

	def ArbRound(self, val, scale, minVal):
		if val < 1.0: 
			p = 4
			dPos = str(1 / val).index('.')	
			b = round(math.pow(.1, dPos), 10)		
		elif val < 2.0: 
			p = 2
			b = .25
		elif val < 5.0: 
			p =  1
			b = 1			
		elif val < 10.0: 
			p = 1
			b = 1
		else:
			p = 1
			dPos = str(val).index('.')	
			b = math.pow(10, dPos - 1)
	
		vRange = self.arbRound(val, p, b)
		minVal = self.arbRound(minVal, p, b)

		lessThanOne = False	
		if val < 1.0:
			val = 1.0 / val
			lessThanOne = True

		intLessThanOne = int(lessThanOne)
		# scale the value from 1. to 10.
		decPos = str(val).index('.')	
		decMult = math.pow(10, decPos + [0, 1][intLessThanOne])
		decMult *= .1
		val = val / decMult

		# get the scale factor
		if val < 2.0: val = .1
		elif val < 5.0: val =  .2
		elif val < 10.0: val =  .4

		if lessThanOne: 
			val /= decMult
			val *= scale *  4
			self.setValueLabels(minVal, val, vRange)
			return val

		val *= decMult
		val *= scale

		self.setValueLabels(minVal, val, vRange)

		return val

	def setValueLabels(self, minVal, val, vRange):
		self.labelVals = []
		self.numValueLabels = int((vRange * 2) / val + 2)
		for i in range(self.numValueLabels):
			posY = vMath.rangeValue(
				minVal,
				*self.keyframerComp.KeysViewVertRange,
				0,
				self.keysViewComp.height	
			)
			minVal = round(minVal, 4)
			self.labelVals.append((minVal, posY))
			minVal += val * .5

		for i, labelComp in enumerate(self.valueLabelComps):
			if i < self.numValueLabels:
				self.valueLabelPars[i][0].val = self.labelVals[i][0]
				self.valueLabelPars[i][1].val = self.labelVals[i][1]
				self.valueLabelPars[i][2].val = True
			else:
				self.valueLabelPars[i][2].val = False


class TimerBarExt(object):
	# TODO implement better method - bring into TimeGraph
	# get all values in one step in Numpy	
	def __init__(self, ownerComp):
		self.ownerComp = ownerComp
		self.keyframerComp = ownerComp.parent.Keyframer
		self.viewComp = self.keyframerComp.op('view')
		self.timeGraphComp = self.viewComp.op('timeGraph')
		self.MinRatio = 1.0
		self.PrevMinRatio = 1.0
		self.Setup()
		self.Position()

	def Position(self):
		try:
			self.NumBeats = math.ceil(self.keyframerComp.LengthSeconds * 2)

			totalWidth = self.timeGraphComp.par.Endbound
			width = totalWidth / self.NumBeats
			widthMin = 20 * self.timeGraphComp.par.Beatlinesscalefactor
			self.MinRatio = widthMin / width
			self.MinRatio = math.pow(2, 
								math.ceil(math.log(self.MinRatio) / math.log(2)))
			self.MinRatio = max(1, self.MinRatio)
			for beat in range(int(self.NumBeats / self.MinRatio) + 1):
				pos = width * self.MinRatio * beat
				self.ownerComp.op('label' + str(beat)).par.x = pos
			if self.MinRatio != self.PrevMinRatio:
				self.Setup()
			self.PrevMinRatio = self.MinRatio
		except:
			pass

	def Setup(self):
		self.SetupSeconds()

	def SetupSeconds(self):
		try:
			self.NumBeats = math.ceil(self.keyframerComp.LengthSeconds * 2)
			for beat in range(self.NumBeats + 1):
				Beat = beat * self.MinRatio	
				label = str(Beat / 2)
				labelOP = self.ownerComp.op('label' + str(beat))
				labelOP.op('text').par.text = label
				if Beat <= self.NumBeats:
					labelOP.par.display = 1
				else:
					labelOP.par.display = 0
		except:
			pass

	def Scrub(self):
		me.fetch('AnimCOMP').par.cuepoint = (self.ownerComp.panel.u * 4 *
						op.LM.fetch('TICKS_PER_BEAT') * me.fetch('LengthBars'))
