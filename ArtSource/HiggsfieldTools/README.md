# Pala y regadera — Higgsfield 3D Jutsu

Modelos creados específicamente para Garden Maniac con Python/Blender ejecutado por el plugin Higgsfield **3D Jutsu**. Son mallas 3D exportables, no imágenes simulando objetos 3D ni modelos descargados de un catálogo.

- Proyecto: https://higgsfield.ai/3d-jutsu/c789d61f-50c2-4ee4-8711-2a8ff593eca2
- Revisión entregada: 1, operación `build-tools-001`.
- `GardenTools.blend`: escena Blender editable con materiales, cámara y luces.
- `GardenTools.glb`: exportación portable PBR de la escena.
- `create_tools.py`: fuente del modelado; requiere el entorno de Higgsfield con `bpy` y `artifacts`.
- `preview.png`: render de verificación desde Blender.
- `import_report.json`: medidas y rutas obtenidas al importar en Unreal.

## Modelos

La pala tiene hoja de acero cóncava, apoyos para el pie, manguito con remaches, mango de madera y empuñadura en D con agarre oscuro. Aproximadamente 1,23 metros y 5.440 triángulos.

La regadera tiene cuerpo hueco de esmalte verde, borde y base de latón, asa con agarre de madera, emblema vegetal y roseta con detalles oscuros de perforación. Aproximadamente 89 × 37 × 47 cm y 27.368 triángulos. Las perforaciones son detalles de superficie, no agujeros booleanos. Es una malla de primera persona para prototipo; pueden crearse LODs y reducir la roseta para producción.

Materiales PBR de colores sólidos, sin dependencia de texturas externas. Las piezas se conservan como grupos de vértices con nombres dentro de las dos mallas. No hay rig, manos ni animación esquelética: golpe e inclinación se animan por código en Unreal.

## Integración

Mallas: `/Game/GardenManiac/Tools/GardenTools/StaticMeshes/SM_GM_Shovel` y `SM_GM_WateringCan`. Materiales en la carpeta hermana `Materials`.

`AGardenPlayer` adjunta las dos mallas a la cámara, sin colisión y visibles solo para su propietario. F equipa la pala, R equipa la regadera, y los números o rueda vuelven a semillas ocultando ambas. Clic produce el golpe; mantener clic inclina la regadera. El socket `WaterSpout` identifica el origen de la traza de agua. El efecto de agua continúa siendo provisional.

Para reimportar, ejecutar `Scripts/import_higgsfield_tools.py` y después `Scripts/configure_tool_meshes.py` con el commandlet Python de Unreal. El segundo script desactiva Nanite para estas piezas pequeñas y configura el socket. Se habilitaron PythonScriptPlugin y EditorScriptingUtilities solo para el editor; no se requiere Higgsfield al ejecutar el juego.

Prueba funcional: iniciar una instancia independiente con `-game -GardenAutomation -ExecCmds=Garden.VerifyTools`. Comprueba equipar, ocultar, daño con pala y recuperación de hidratación; termina automáticamente. Con renderizado produce `Saved/Screenshots/GardenShovel.png` y `GardenWateringCan.png`.

Verificado: compilación Development Editor Win64; importación de ambos assets; prueba `GardenTools: shovel=1 watering=1 unequip=1 PASS`; revisión de render Blender y capturas dentro de Unreal. Una luz suave, limitada al canal de iluminación de las herramientas, permite ver los materiales dentro de la casa sin iluminar el escenario.
