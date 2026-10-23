import unreal, os, json
root = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
source = os.path.join(root, 'ArtSource/HiggsfieldTools/GardenTools.glb')
task = unreal.AssetImportTask()
task.filename=source
task.destination_path='/Game/GardenManiac/Tools'
task.automated=True
task.replace_existing=True
task.save=True
# Use Unreal's GLTF pipeline, preserving its scene/type subfolders and baked placement.
# The first-person components position the imported meshes by their bounds.
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
assets=unreal.EditorAssetLibrary.list_assets('/Game/GardenManiac/Tools',recursive=True,include_folder=False)
report=[]
for path in assets:
 obj=unreal.load_asset(path)
 if isinstance(obj,unreal.StaticMesh):
  name=obj.get_name()
  box=obj.get_bounding_box()
  report.append({'path':path,'name':name,'min':str(box.min),'max':str(box.max),'materials':len(obj.static_materials)})
  unreal.EditorAssetLibrary.save_loaded_asset(obj)
with open(os.path.join(root,'ArtSource/HiggsfieldTools/import_report.json'),'w') as f:json.dump(report,f,indent=2)
unreal.log('GARDEN_TOOLS_IMPORT '+json.dumps(report))
assert len(report)==2, 'Expected exactly two imported tool meshes'
