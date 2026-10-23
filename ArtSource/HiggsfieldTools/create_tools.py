import bpy, math
from mathutils import Vector
bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)
def mat(name,color,metal=0,rough=.4):
 m=bpy.data.materials.new(name); m.diffuse_color=(*color,1); m.use_nodes=True
 p=m.node_tree.nodes.get('Principled BSDF'); p.inputs['Base Color'].default_value=(*color,1); p.inputs['Metallic'].default_value=metal; p.inputs['Roughness'].default_value=rough
 return m
steel=mat('Brushed steel',(.28,.37,.4),.8,.3)
edge=mat('Polished edges',(.6,.69,.7),.85,.23)
wood=mat('Warm ash wood',(.46,.22,.065),0,.48)
green=mat('Garden enamel',(.045,.24,.12),.3,.3)
dark=mat('Dark rubber and holes',(.018,.028,.022),0,.65)
brass=mat('Warm brass',(.64,.36,.08),.7,.3)
inside=mat('Interior enamel',(.023,.09,.05),.2,.45)
parts=[]
def finish(o,name,material,bevel=0):
 o.name=name; o.data.materials.append(material)
 if bevel:
  mod=o.modifiers.new('Soft manufactured edges','BEVEL'); mod.width=bevel; mod.segments=3
 for p in o.data.polygons:p.use_smooth=True
 parts.append(o); return o

def rod(name,a,b,r,material,r2=None):
 a,b=Vector(a),Vector(b); d=b-a
 bpy.ops.mesh.primitive_cone_add(vertices=32,radius1=r,radius2=r if r2 is None else r2,depth=d.length,location=(a+b)/2)
 o=bpy.context.object; o.rotation_euler=d.to_track_quat('Z','Y').to_euler()
 return finish(o,name,material,.002)

def tube(name,coords,r,material):
 c=bpy.data.curves.new(name,'CURVE'); c.dimensions='3D'; c.resolution_u=12; c.bevel_depth=r; c.bevel_resolution=3
 s=c.splines.new('BEZIER'); s.bezier_points.add(len(coords)-1)
 for p,co in zip(s.bezier_points,coords):p.co=co; p.handle_left_type='AUTO';p.handle_right_type='AUTO'
 o=bpy.data.objects.new(name,c);bpy.context.collection.objects.link(o);bpy.context.view_layer.objects.active=o;o.select_set(True)
 bpy.ops.object.convert(target='MESH');o=bpy.context.object;o.select_set(False)
 return finish(o,name,material)

def lathe(name,profile,material,n=48):
 verts=[(r*math.cos(2*math.pi*i/n),r*math.sin(2*math.pi*i/n),z) for r,z in profile for i in range(n)]
 faces=[]
 for j in range(len(profile)-1):
  for i in range(n): a=j*n+i;b=j*n+(i+1)%n;faces.append((a,b,b+n,a+n))
 me=bpy.data.meshes.new(name);me.from_pydata(verts,[],faces);me.update();o=bpy.data.objects.new(name,me);bpy.context.collection.objects.link(o)
 return finish(o,name,material)

def join_tool(name,offset):
 global parts
 bpy.ops.object.select_all(action='DESELECT')
 for o in parts:
  bpy.context.view_layer.objects.active=o;o.select_set(True)
  for mod in list(o.modifiers):bpy.ops.object.modifier_apply(modifier=mod.name)
  g=o.vertex_groups.new(name=o.name);g.add(list(range(len(o.data.vertices))),1,'REPLACE')
 bpy.context.view_layer.objects.active=parts[0];bpy.ops.object.join();o=bpy.context.object;o.name=name
 bpy.context.scene.cursor.location=(0,0,0);bpy.ops.object.origin_set(type='ORIGIN_CURSOR');bpy.ops.object.transform_apply(location=False,rotation=True,scale=True)
 o.location=offset;o['source']='Modeled with Higgsfield 3D Jutsu for Garden Maniac';o['units']='metres';parts=[];return o

# Shovel: cupped, pointed blade with rolled shoulders, socket, shaft, D grip.
verts=[];faces=[]
rows=[(.025,.015),(.07,.065),(.15,.13),(.27,.145),(.335,.12)]
for z,w in rows:
 for i in range(9):
  t=(i-4)/4;verts.append((w*t,.034*(1-t*t),z))
for j in range(4):
 for i in range(8):a=j*9+i;faces.append((a,a+1,a+10,a+9))
me=bpy.data.meshes.new('Cupped blade');me.from_pydata(verts,[],faces);me.update();o=bpy.data.objects.new('Cupped blade',me);bpy.context.collection.objects.link(o)
finish(o,'Cupped steel blade',steel,.003);s=o.modifiers.new('Blade thickness','SOLIDIFY');s.thickness=.006
rod('Reinforced socket',(0,.035,.25),(0,.015,.43),.024,steel,r2=.019)
rod('Wood shaft',(0,.015,.4),(0,.015,1.05),.018,wood,r2=.022)
rod('Socket collar',(0,.015,.4),(0,.015,.43),.023,brass)
for x in [-.09,.09]:rod('Boot shoulder',(x-.04,.01,.33),(x+.04,.01,.33),.012,edge)
tube('D grip frame',[(-.018,.015,1.015),(-.075,.015,1.10),(-.075,.015,1.20),(.075,.015,1.20),(.075,.015,1.10),(.018,.015,1.015)],.017,green)
rod('Grip rubber',(-.063,.015,1.19),(.063,.015,1.19),.022,dark)
for x in [-.018,.018]:rod('Socket rivet',(x,-.004,.365),(x,-.015,.365),.007,brass)
shovel=join_tool('SM_GM_Shovel',(-.48,0,0))
# Watering can: hollow body, rolled rims, rear loop, rising spout and perforated rose.
lathe('Hollow enamel body',[(0,.025),(.15,.025),(.173,.045),(.181,.09),(.175,.26),(.15,.33),(.125,.35),(.113,.35),(.113,.338),(.139,.321),(.163,.255),(.169,.09),(.16,.05),(0,.05)],green)
lathe('Bottom rolled seam',[(.168,.034),(.181,.04),(.183,.055),(.176,.065)],brass)
lathe('Open filling rim',[(.113,.337),(.126,.34),(.129,.35),(.124,.362),(.112,.36),(.108,.35),(.113,.337)],brass)
tube('Rear carrying loop',[(-.145,0,.08),(-.28,0,.13),(-.29,0,.33),(-.15,0,.42),(-.08,0,.34)],.018,green)
rod('Wood handle grip',(-.28,0,.16),(-.285,0,.29),.023,wood)
rod('Rising tapered spout',(.13,0,.09),(.45,0,.38),.05,green,r2=.019)
rod('Spout brass collar',(.42,0,.353),(.465,0,.395),.025,brass)
a=Vector((.47,0,.4)); normal=Vector((.74,0,.67)).normalized();b=a+normal*.042
rod('Rose cone',a,b,.03,brass,r2=.082)
rod('Rose perforation plate',b,b+normal*.008,.08,edge)
axis=Vector((0,1,0));other=normal.cross(axis)
for rr,count in [(0,1),(.025,8),(.051,14),(.068,18)]:
 for i in range(count):
  q=b+normal*.009+rr*(axis*math.cos(2*math.pi*i/count)+other*math.sin(2*math.pi*i/count))
  rod('Rose recessed hole',q,q+normal*.001,.003,dark)
# Raised garden leaf emblem facing the camera.
tube('Emblem stem',[(0,-.177,.12),(.012,-.181,.18),(.01,-.175,.24)],.004,brass)
for x,z,ang in [(-.025,.18,-.6),(.037,.213,.6)]:
 bpy.ops.mesh.primitive_uv_sphere_add(segments=16,ring_count=8,location=(x,-.178,z));o=bpy.context.object;o.scale=(.027,.007,.049);o.rotation_euler[1]=ang;finish(o,'Raised leaf badge',brass)
can=join_tool('SM_GM_WateringCan',(.20,0,0))
scene=bpy.context.scene;scene.unit_settings.system='METRIC';scene.unit_settings.scale_length=1
world=bpy.data.worlds.new('Soft blue studio');scene.world=world;world.use_nodes=True;world.node_tree.nodes['Background'].inputs[0].default_value=(.085,.11,.14,1);world.node_tree.nodes['Background'].inputs[1].default_value=.5
bpy.ops.object.camera_add(location=(2.0,-3.4,1.9));cam=bpy.context.object;cam.name='DeliveryCamera';cam.rotation_euler=(Vector((0,0,.59))-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.type='ORTHO';cam.data.ortho_scale=1.7;scene.camera=cam
for name,loc,energy,color,size in [('Warm key',(-2,-3,3),420,(1,.85,.68),1.0),('Cool fill',(2,-1,2),260,(.65,.8,1),1.0),('Rim',(0,2,3),500,(1,1,1),.8)]:
 bpy.ops.object.light_add(type='POINT',location=loc);l=bpy.context.object;l.name=name;l.data.energy=energy;l.data.color=color;l.data.shadow_soft_size=size
scene.render.engine='BLENDER_EEVEE';scene.render.resolution_x=1000;scene.render.resolution_y=850;scene.render.resolution_percentage=100
scene.render.image_settings.file_format='PNG'
result={'models':[{'name':o.name,'dimensions':list(o.dimensions),'vertices':len(o.data.vertices),'polygons':len(o.data.polygons)} for o in [shovel,can]]}
target=artifacts.file(name='garden_tools_preview.png',media_type='image/png');scene.render.filepath=target.path;bpy.ops.render.render(write_still=True);target.publish()
