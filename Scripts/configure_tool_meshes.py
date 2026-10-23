import unreal
base='/Game/GardenManiac/Tools/GardenTools/StaticMeshes/'
for name in ['SM_GM_Shovel','SM_GM_WateringCan']:
 mesh=unreal.load_asset(base+name)
 assert mesh, name
 settings=mesh.get_editor_property('nanite_settings')
 settings.set_editor_property('enabled',False)
 mesh.set_editor_property('nanite_settings',settings)
 if name=='SM_GM_WateringCan' and not mesh.find_socket('WaterSpout'):
  socket=unreal.new_object(unreal.StaticMeshSocket,outer=mesh)
  socket.set_editor_property('socket_name','WaterSpout')
  socket.set_editor_property('relative_location',unreal.Vector(70.8,0,43.4))
  mesh.add_socket(socket)
 unreal.EditorAssetLibrary.save_loaded_asset(mesh)
 unreal.log('GARDEN_TOOL_READY '+mesh.get_path_name())
