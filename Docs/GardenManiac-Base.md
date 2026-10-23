# Garden Maniac — base del prototipo

## Dirección

Defensa de una casa en primera persona, con jardín estratégico y mantenimiento activo de plantas. Estética objetivo: cartoon-realista, siluetas legibles, materiales sencillos y luz cálida. Este prototipo usa formas de `/Engine/BasicShapes`; todavía no representa el acabado artístico final.

El jugador prepara defensas, combate con pala y mantiene la hidratación mientras avanza una horda. Cada día termina cuando se han generado y resuelto todos sus zombis. La siguiente preparación entrega recursos y permite ampliar defensas.

## Arranque

1. Compilar `GardenManiacEditor`, configuración Development, Win64, Unreal 5.6.
2. Si Unreal estaba abierto al compilar, guardar el trabajo y reiniciar el editor para cargar las nuevas clases nativas.
3. Abrir `/Game/FirstPerson/Lvl_FirstPerson` y pulsar **Play**.
4. Su `BP_FirstPersonGameMode`, derivado de `GardenManiacGameMode`, genera la casa de prueba y coloca al jugador dentro.

El escenario aparece durante Play a 120 metros del origen para conservar el mapa original. No se han reemplazado mapas ni assets binarios. Desactivar `Enable Garden Prototype` en el GameMode permite volver al comportamiento del template. Las variantes Shooter/Horror conservan sus propios GameModes.

Es un prototipo local de un jugador. No tiene guardado, replicación ni soporte de mando/móvil. Para este primer control de escritorio, WASD y números se consultan directamente; migrar a Input Actions configurables antes de producir opciones de controles.

## Controles

| Acción | Control |
|---|---|
| Caminar / mirar | WASD / ratón |
| Saltar | Espacio |
| Seleccionar una de ocho semillas | 1–8 o rueda |
| Equipar / guardar pala | F |
| Equipar / guardar regadera | R |
| Plantar / golpear / retirar planta | Clic izquierdo |
| Regar de forma continua | Mantener clic izquierdo |
| Mejorar jardín | E |
| Reiniciar tras derrota | Enter |

Las semillas se plantan apuntando a una casilla desocupada a un máximo de 650 cm. La pala alcanza 240 cm y retira sin devolución de recursos. La regadera es ilimitada en esta fase. La pala y la regadera ya tienen mallas propias creadas con Higgsfield 3D Jutsu, visibles en primera persona. El golpe y la inclinación al regar se animan por código; aún no hay manos ni rig. Fuentes y detalles en `ArtSource/HiggsfieldTools/README.md`.

## Reglas iniciales, todas provisionales

- 250 de sol y 500 de integridad de casa al empezar; 40 segundos de preparación inicial.
- Ingreso pasivo de 10 de sol cada 8 segundos, más 5 por nivel de jardín.
- Girasoles: 25 de sol cada 8 segundos a hidratación completa. La variante mayor duplica producción.
- Plantas pierden 1,3 puntos de agua por segundo. Con agua baja, ataque y producción se ralentizan; a cero se detienen. La regadera aporta 40 puntos por segundo aproximadamente.
- Las defensivas absorben daño y permanecen como barreras aunque se sequen. No atacan.
- Patio A desde el día 1; patio B desde el 3; tejado desde el 5. Se puede recorrer la casa y subir antes del desbloqueo; plantar en sectores bloqueados no está permitido.
- Patio B tiene tres carriles, el central acuático. En esta fase los zombis atraviesan ese carril con movimiento provisional, sin natación específica.
- Tejado con escalera interior y llegada de zombis por un trayecto vertical exterior.
- Zombis normales, especiales rápidos desde el segundo día y un boss cada cinco días. Los especiales y bosses tienen estadísticas distintas, pero aún no habilidades exclusivas.
- Tres mejoras con E: costes de 150, 300 y 450; aumentan ingreso y capacidad de integridad de casa, y reparan hasta 150 puntos. Aún sin cambios visuales de construcción.
- No hay final de campaña: las oleadas continúan hasta la derrota, con cantidad limitada a 30 por día.

## Arquitectura

| Tipo | Responsabilidad |
|---|---|
| `AGardenPlayer` | Cámara, movimiento, ocho ranuras, selección de herramienta, interacción y recargas |
| `UGardenSeed` | Nombre, clase plantada, coste, recarga, compatibilidad con agua, color y multiplicadores |
| `APlanta` | Vida, hidratación, rendimiento y ciclo de producción |
| `APlantaOfensiva` | Evento extensible `Atacar`, búsqueda por sector/carril y línea de visión |
| `APlantaDefensiva` | Barrera de alta vida |
| `APlantaMiscelanea` | Evento extensible `EjecutarEspecial` |
| `APlantaGirasol` | Ejemplo de miscelánea productora de recursos |
| `AZombie` | Vida, ataque a plantas, ruta, daño a casa y recompensa |
| `AZombieNormal / Especial / Boss` | Familias para estadísticas y futuras habilidades |
| `AGardenPlot` | Casilla, sector, agua y referencia débil a la planta |
| `AGardenDirector` | Economía, días, oleadas, desbloqueos y generación del escenario |
| `AGardenHUD` | Ranuras, estado de casa, fase, hidratación y mensajes |

Se eligen clases padre, en vez de interfaces `IZombie`/`IPlanta`, porque estas familias necesitan compartir estado, componentes y lógica. Una interfaz no aporta esa implementación. Se puede añadir después una interfaz transversal de daño/interacción si aparecen actores ajenos a estas familias.

Los ataques ofensivos son instantáneos con una línea verde de depuración, no proyectiles físicos. Los zombis usan rutas explícitas, sin NavMesh; atacan plantas cercanas en su carril y dañan la casa al llegar. No persiguen al jugador, pero pueden agarrarlo al acercarse: tiene tres segundos para recibir ayuda de una planta o pierde la partida. Son decisiones de prototipo, no arquitectura definitiva para enemigos que exploren habitaciones.

## Añadir contenido

### Zombi

Crear un Blueprint hijo de `ZombieNormal`, `ZombieEspecial` o `ZombieBoss`. Ajustar `MaxHealth`, `Speed`, `AttackDamage`, `AttackInterval` y `Reward`; sustituir la geometría del componente `Mesh`. Implementar `HabilidadEspecial(DeltaSeconds)` para habilidades nuevas. Asignar la clase a `NormalClass`, `SpecialClass` o `BossClass` de un Blueprint hijo de `GardenDirector`.

### Planta y semilla

Crear un Blueprint hijo de `PlantaOfensiva`, `PlantaDefensiva` o `PlantaMiscelanea`. En la ofensiva sobrescribir `Atacar`; en la miscelánea, `EjecutarEspecial`. Crear un Data Asset de tipo `GardenSeed`, asignar `PlantClass` y estadísticas. Añadirlo al índice deseado de `SeedLoadout` del Blueprint del director; los ocho índices vacíos usan los valores de prototipo. Los multiplicadores de semilla funcionan independientemente del número de ranura.

En `BP_FirstPersonGameMode`, asignar ese director a `GardenDirectorClass`. También se permite sustituir `GardenPlayerClass`. Los eventos son `BlueprintNativeEvent`: pueden ampliarse en Blueprint conservando una implementación C++.

Imán y plantorcha están previstos como misceláneas, no implementados. Para el imán conviene definir primero equipo/armadura desmontable de zombis. Para plantorcha, introducir proyectiles con atributos transformables, evitando convertirla en otra planta que simplemente dispara.

## Próximos hitos

1. Probar y ajustar el bucle: tiempos, costes, agua, distancias y presión de oleadas.
2. Proyectiles reales, animación/feedback de pala, regadera visible y enemigos con ataques al jugador.
3. Oleadas y campañas mediante Data Assets, nuevas habilidades y enemigos acuáticos/escaladores propios.
4. Imán, plantorcha y mejoras físicas de la casa; guardado de progreso.
5. Arte cartoon-realista: bloquear proporciones y siluetas primero, luego sustituir primitivas por modelos y animaciones. Mantener contratos de gameplay al integrar recursos generados.

## Validación

Verificado en esta entrega: compilación Development Editor Win64 correcta; `GardenManiac.Prototype.Systems` aprobado sin errores ni advertencias; arranque de `/Game/FirstPerson/Lvl_FirstPerson` en modo juego sin renderizado correcto. Informe: `Saved/Automation/GardenPrototype/index.json`. Registro de arranque: `Saved/Logs/GardenPrototypeStartup.log`.

La entrega actual de herramientas utiliza una compilación normal `UnrealEditor-GardenManiac.dll`. Las compilaciones con sufijo se usaron durante las primeras pruebas cuando el editor estaba abierto. Guardar y reiniciar el editor después de actualizar clases nativas.

Prueba disponible: `GardenManiac.Prototype.Systems` en Automation. Cubre compra inválida, desbloqueo por días, sequía y recuperación de producción, ataque por carril/sector, recompensa única, mejoras y detención al perder.

Revisión manual pendiente dentro de Play: recorrer ambas puertas y toda la escalera; plantar y retirar en los ocho slots; defender hasta día 5; comprobar claridad del HUD y balance real. La compilación y las pruebas automáticas no sustituyen esta revisión visual/jugable.


## Extensiones y rescate

La guía Docs/Crear-Zombies-y-Plantas.md describe cómo crear Blueprints hijos, importar modelos, conectar Animation Blueprints y registrar semillas y familias de zombis. Incluye los eventos de agarre y las limitaciones actuales de rutas, animaciones y proyectiles.

Los disparos atraviesan plantas aliadas y al jugador, pero respetan paredes y otros enemigos. Un impacto de planta sobre el captor libera al jugador y otorga dos segundos de protección contra otro agarre. La cuenta atrás del HUD dura tres segundos; al expirar, Enter permite reiniciar.
