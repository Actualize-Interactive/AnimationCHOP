def onCellChange(dat, cells, prev):
	"""
	Called when cells in the keyframes table are manually changed.
	Updates the corresponding keyframe in the AnimationCHOP.
	"""
	try:
		# If more than one cell changed, it's likely a bulk operation - don't process
		if len(cells) > 1:
			print(f"onCellChange: Skipping bulk operation ({len(cells)} cells changed)")
			return
		
		# Skip changes to index column (column 0) since it's read-only
		cell = cells[0]
		if cell.col == 0:
			print(f"onCellChange: Skipping index column change")
			return
		
		# Get the AnimationCHOP node
		anim_chop = op('Animationchop1')
		if not anim_chop:
			print("ERROR: AnimationCHOP node 'Animationchop1' not found")
			return
		
		print(f"onCellChange: Processing single cell change")
		
		row = cell.row
		col = cell.col
		val = cell.val
		
		# Skip header row
		if row == 0:
			return
			
		print(f"Cell changed: row={row}, col={col}, value='{val}'")
		
		# Get the keyframe index (row - 1 to account for header)
		keyframe_index = row - 1
		
		# Get current row data to reconstruct the keyframe
		try:
			index_val = dat[row, 0].val if dat[row, 0].val else keyframe_index
			time_val = float(dat[row, 1].val) if dat[row, 1].val else 0.0
			value_val = float(dat[row, 2].val) if dat[row, 2].val else 0.0
			
			# Convert mode string to mode number with validation
			mode_str = dat[row, 3].val if dat[row, 3].val else "smooth"
			tangent_modes_reverse = {
				"flat": 0, 
				"linear": 1, 
				"constant": 2,
				"smooth": 3, 
				"manual": 4, 
				"broken": 5
			}
			
			if mode_str not in tangent_modes_reverse:
				print(f"WARNING: Invalid mode string '{mode_str}', defaulting to 'smooth'")
				mode_str = "smooth"
				# Update the cell with the corrected value
				dat[row, 3] = mode_str
			
			mode_val = tangent_modes_reverse[mode_str]
			print(f"Mode conversion: '{mode_str}' -> {mode_val}")
			
			# Parse handle values with better error handling
			try:
				in_time_str = dat[row, 4].val
				if in_time_str and in_time_str != "N/A":
					in_time = float(in_time_str)
				else:
					in_time = time_val - 0.1
			except (ValueError, TypeError):
				in_time = time_val - 0.1
				print(f"WARNING: Invalid in_time value, using default {in_time}")
			
			try:
				in_val_str = dat[row, 5].val
				if in_val_str and in_val_str != "N/A":
					in_val = float(in_val_str)
				else:
					in_val = value_val
			except (ValueError, TypeError):
				in_val = value_val
				print(f"WARNING: Invalid in_val value, using default {in_val}")
			
			try:
				out_time_str = dat[row, 6].val
				if out_time_str and out_time_str != "N/A":
					out_time = float(out_time_str)
				else:
					out_time = time_val + 0.1
			except (ValueError, TypeError):
				out_time = time_val + 0.1
				print(f"WARNING: Invalid out_time value, using default {out_time}")
			
			try:
				out_val_str = dat[row, 7].val
				if out_val_str and out_val_str != "N/A":
					out_val = float(out_val_str)
				else:
					out_val = value_val
			except (ValueError, TypeError):
				out_val = value_val
				print(f"WARNING: Invalid out_val value, using default {out_val}")
			
			print(f"Updating keyframe {keyframe_index}: time={time_val}, value={value_val}, mode={mode_val} ({mode_str})")
			print(f"  Handles: in=({in_time:.3f}, {in_val:.3f}), out=({out_time:.3f}, {out_val:.3f})")
			
			# Update the keyframe in the AnimationCHOP
			success = anim_chop.set_keyframe_at_time(
				"keyframe_showcase",
				time_val,
				value_val,
				mode_val,
				in_time,
				in_val,
				out_time,
				out_val
			)
			
			if success:
				print(f"✓ Successfully updated keyframe at time {time_val}")
			else:
				print(f"✗ Failed to update keyframe at time {time_val}")
				
		except ValueError as e:
			print(f"✗ Error parsing values for row {row}: {e}")
		except Exception as e:
			print(f"✗ Error updating keyframe for row {row}: {e}")
			
	except Exception as e:
		print(f"ERROR in onCellChange: {e}")
		import traceback
		traceback.print_exc()

def onSizeChange(dat):
	"""
	Called when the table size changes (rows added/removed).
	Rebuilds all keyframes from the table to ensure consistency.
	"""
	try:
		# Get the AnimationCHOP node
		anim_chop = op('Animationchop1')
		if not anim_chop:
			print("ERROR: AnimationCHOP node 'Animationchop1' not found")
			return
			
		print(f"onSizeChange: Table now has {dat.numRows} rows, {dat.numCols} cols")
		
		# Skip if we only have header row or no rows
		if dat.numRows <= 1:
			print("Table only has header row or is empty - clearing all keyframes")
			# Clear all keyframes if table is empty
			try:
				anim_chop.clear_channels()
				anim_chop.create_channel("keyframe_showcase")
			except Exception as e:
				print(f"Failed to clear/recreate channel: {e}")
			return
		
		# Clear the channel and recreate it to start fresh
		try:
			anim_chop.clear_channels()
			channel = anim_chop.create_channel("keyframe_showcase")
			if not channel:
				print("ERROR: Failed to recreate keyframe_showcase channel")
				return
		except Exception as e:
			print(f"ERROR: Failed to clear/recreate channel: {e}")
			return
		
		print("Rebuilding all keyframes from table data...")
		
		# Define mode mappings for validation
		tangent_modes_reverse = {
			"flat": 0, 
			"linear": 1, 
			"constant": 2,
			"smooth": 3, 
			"manual": 4, 
			"broken": 5
		}
		tangent_modes = {
			0: "flat", 
			1: "linear", 
			2: "constant",
			3: "smooth", 
			4: "manual", 
			5: "broken"
		}
		
		try:
			# First pass: calculate appropriate time values for empty cells
			for row in range(1, dat.numRows):
				# Check if time cell is empty and needs a calculated value
				time_cell = dat[row, 1]  # time column
				if not time_cell.val:
					# Calculate time based on surrounding keyframes
					index_val = row - 1  # keyframe index
					
					# Get times from surrounding rows (if they exist and have values)
					prev_time = None
					next_time = None
					
					# Look for previous row with time value
					for prev_row in range(row - 1, 0, -1):
						if dat[prev_row, 1].val:
							prev_time = float(dat[prev_row, 1].val)
							break
					
					# Look for next row with time value
					for next_row in range(row + 1, dat.numRows):
						if dat[next_row, 1].val:
							next_time = float(dat[next_row, 1].val)
							break
					
					# Calculate appropriate time value
					if prev_time is not None and next_time is not None:
						# Insert in the middle of surrounding keyframes
						calculated_time = (prev_time + next_time) / 2.0
						print(f"Row {row}: Calculated time {calculated_time:.1f} (between {prev_time:.1f} and {next_time:.1f})")
					elif prev_time is not None:
						# Only have previous time, add 0.5 seconds
						calculated_time = prev_time + 0.5
						print(f"Row {row}: Calculated time {calculated_time:.1f} (after {prev_time:.1f})")
					elif next_time is not None:
						# Only have next time, subtract 0.5 seconds
						calculated_time = max(0.0, next_time - 0.5)
						print(f"Row {row}: Calculated time {calculated_time:.1f} (before {next_time:.1f})")
					else:
						# No surrounding times, use index-based spacing
						calculated_time = index_val * 0.5
						print(f"Row {row}: Calculated time {calculated_time:.1f} (index-based)")
					
					# Update the time cell with calculated value
					dat[row, 1] = calculated_time
			
			# Second pass: process each row and create keyframes
			for row in range(1, dat.numRows):
				try:
					# Get row data with defaults for empty cells
					index_val = row - 1  # Calculate correct index
					time_val = float(dat[row, 1].val) if dat[row, 1].val else index_val * 0.5
					value_val = float(dat[row, 2].val) if dat[row, 2].val else 10.0 + 5.0 * (index_val % 4)
					
					# Convert mode string to mode number with validation
					mode_str = dat[row, 3].val if dat[row, 3].val else "smooth"
					
					if mode_str not in tangent_modes_reverse:
						print(f"WARNING: Invalid mode string '{mode_str}' in row {row}, defaulting to 'smooth'")
						mode_str = "smooth"
						mode_val = 3
					else:
						mode_val = tangent_modes_reverse[mode_str]
					
					# Ensure consistency between mode string and number
					mode_name = tangent_modes.get(mode_val, "smooth")
					if mode_name != mode_str:
						print(f"WARNING: Mode mismatch in row {row}: '{mode_str}' != '{mode_name}', correcting to '{mode_name}'")
						mode_str = mode_name
						mode_val = tangent_modes_reverse[mode_name]
					
					# Get tangent handle values with defaults (column indices: 4=in_time, 5=in_val, 6=out_time, 7=out_val)
					if dat[row, 4].val and dat[row, 4].val != "N/A":
						try:
							in_time = float(dat[row, 4].val)
						except (ValueError, TypeError):
							in_time = time_val - 0.1
					else:
						in_time = time_val - 0.1
					
					if dat[row, 5].val and dat[row, 5].val != "N/A":
						try:
							in_val = float(dat[row, 5].val)
						except (ValueError, TypeError):
							in_val = value_val
					else:
						in_val = value_val
					
					if dat[row, 6].val and dat[row, 6].val != "N/A":
						try:
							out_time = float(dat[row, 6].val)
						except (ValueError, TypeError):
							out_time = time_val + 0.1
					else:
						out_time = time_val + 0.1
					
					if dat[row, 7].val and dat[row, 7].val != "N/A":
						try:
							out_val = float(dat[row, 7].val)
						except (ValueError, TypeError):
							out_val = value_val
					else:
						out_val = value_val
					
					print(f"Creating keyframe {index_val}: time={time_val}, value={value_val}, mode={mode_val} ({mode_str})")
					
					# Add the keyframe to the AnimationCHOP
					success = anim_chop.set_keyframe_at_time(
						"keyframe_showcase",
						time_val,
						value_val,
						mode_val,
						in_time,
						in_val,
						out_time,
						out_val
					)
					
					if success:
						print(f"✓ Rebuilt keyframe {index_val}: time={time_val}, value={value_val}")
					else:
						print(f"✗ Failed to rebuild keyframe {index_val}")
					
					# Update the table row to ensure it has correct values (especially index and mode)
					dat[row, 0] = index_val  # Fix index column
					
					# Fill in any missing data with defaults and ensure consistency
					if not dat[row, 1].val:
						dat[row, 1] = time_val
					if not dat[row, 2].val:
						dat[row, 2] = value_val
					if not dat[row, 3].val or dat[row, 3].val not in tangent_modes_reverse:
						dat[row, 3] = mode_str
					if not dat[row, 4].val or dat[row, 4].val == "N/A":
						dat[row, 4] = f"{in_time:.3f}"
					if not dat[row, 5].val or dat[row, 5].val == "N/A":
						dat[row, 5] = f"{in_val:.3f}"
					if not dat[row, 6].val or dat[row, 6].val == "N/A":
						dat[row, 6] = f"{out_time:.3f}"
					if not dat[row, 7].val or dat[row, 7].val == "N/A":
						dat[row, 7] = f"{out_val:.3f}"
						
				except ValueError as e:
					print(f"✗ Error parsing row {row}: {e}")
					# Set error indicators in the row
					dat[row, 0] = f"ERR"
					
				except Exception as e:
					print(f"✗ Error processing row {row}: {e}")
					dat[row, 0] = f"ERR"
		
		finally:
			# Re-enable callbacks
			dat.par.active = True
			
		print(f"✓ Size change processing complete - rebuilt {dat.numRows - 1} keyframes")
			
	except Exception as e:
		print(f"ERROR in onSizeChange: {e}")
		import traceback
		traceback.print_exc()
		
		# Make sure to re-enable callbacks even if there was an error
		try:
			dat.par.active = True
		except:
			pass
