-- Transient script for executing various one-off console commands through 'f:Cmd'.
-- Modify as desired, no matter it's current contents; If it's anything worth keeping, it doesn't belong in here.


local ent = -1
local cleanup = false


print("Working tests")
if true then
	print("Test 1")
	ent = scene.CreateEntity()
	scene.SetComponent(ent, "Transform", {})
	scene.SetComponent(ent, "Collider", {})
	
	if cleanup then
		scene.RemoveEntity(ent)
	end
end
	
if true then
	print("Test 2")
	ent = scene.CreateEntity()
	scene.SetComponent(ent, "Collider", {})
	scene.SetComponent(ent, "Transform", {})
		
	if cleanup then
		scene.RemoveEntity(ent)
	end
end
	
if true then
	print("Test 3")
	ent = scene.CreateEntity()
	scene.SetComponent(ent, "Collider", {})
	scene.SetComponent(ent, "Collider", {})
		
	if cleanup then
		scene.RemoveEntity(ent)
	end
end
	
if true then
	print("Test 4")
	ent = scene.CreateEntity()
	scene.SetComponent(ent, "Transform", {})
	scene.SetComponent(ent, "Transform", {})
		
	if cleanup then
		scene.RemoveEntity(ent)
	end
end


print("Not working tests")
if true then
	print("Test 1")
	ent = scene.CreateEntity()
	scene.SetComponent(ent, "Collider", {})
	scene.SetComponent(ent, "Collider", {})
	scene.SetComponent(ent, "Transform", {})
		
	if cleanup then
		scene.RemoveEntity(ent)
	end
end
	
if true then
	print("Test 2")
	ent = scene.CreateEntity()
	scene.SetComponent(ent, "Transform", {})
	scene.SetComponent(ent, "Collider", {})
	scene.SetComponent(ent, "Collider", {})
		
	if cleanup then
		scene.RemoveEntity(ent)
	end
end

if true then
	print("Test 3")
	ent = scene.CreateEntity()
	scene.SetComponent(ent, "Collider", {})
	scene.SetComponent(ent, "Transform", {})
	scene.SetComponent(ent, "Collider", {})
		
	if cleanup then
		scene.RemoveEntity(ent)
	end
end
