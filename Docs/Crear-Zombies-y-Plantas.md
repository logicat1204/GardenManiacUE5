# Crear tus propios zombis y plantas

La base actual usa **clases padre C++**, no interfaces de Unreal `IZombie` o `IPlanta`. Los Blueprints hijos heredan vida, hidratación, daño y demás comportamiento. No necesitas modificar las clases originales para crear una especie.

## Organización recomendada

Crea una carpeta por especie: `Content/GardenManiac/Zombies/ZombieJardinero` o `Content/GardenManiac/Plants/LanzabroteAzul`. Dentro guarda `Meshes`, `Materials`, `Animations` y el Blueprint. Para plantas, guarda también su Data Asset de semilla. Evita editar directamente assets del template.

## Tu primer zombi

1. En Content Browser, **Blueprint Class → All Classes**, busca `ZombieNormal`, `ZombieEspecial` o `ZombieBoss`. Crea, por ejemplo, `BP_ZombieJardinero`.
2. En Class Defaults ajusta `MaxHealth`, `Speed`, `AttackDamage`, `AttackInterval`, `Reward`, `Can Grab` y `Grab Range`. La familia cambia estadísticas iniciales; no aporta automáticamente una habilidad exclusiva.
3. Para probarlo en las oleadas, crea un Blueprint hijo de `GardenDirector`, llamado `BP_GardenDirector_Custom`. Asigna tu clase a `NormalClass`, `SpecialClass` o `BossClass`.
4. Abre `BP_FirstPersonGameMode` y asigna `BP_GardenDirector_Custom` a `GardenDirectorClass`. No basta con colocar el nuevo director en el mapa: el GameMode crea el director de la partida.

Actualmente cada familia tiene un único campo de clase. Para mezclar varias especies normales en una misma oleada, el siguiente paso recomendable es un Data Asset de oleada con una lista de clases, cantidades, sectores y tiempos; esa lista todavía no está implementada.

## Modelos y colisión

Para un modelo sin animación puedes reemplazar el Static Mesh del componente heredado `Mesh`, ajustando tamaño y colisión. Ten presente que el prototipo recolorea su primer material al aparecer; en plantas también lo hace al plantar.

Para conservar tus materiales y usar personajes animados, recomiendo separar representación y colisión:

1. Conserva `Mesh` como volumen sencillo de colisión, con su colisión activa.
2. Ocúltalo con **Set Visibility(false)** sin propagar a los hijos. No uses `Set Actor Hidden In Game`, porque ocultaría también tu modelo.
3. Añade un componente visual propio: **Static Mesh** o **Skeletal Mesh**. Asigna ahí tu modelo y materiales y usa `NoCollision` para evitar colisiones duplicadas.
4. Ajusta la transformación relativa del componente visual. El `Mesh` raíz del prototipo tiene escala no uniforme: activa escala absoluta en el componente visual o compensa esa escala. Comprueba el tamaño en centímetros y que los pies apoyen sobre el suelo.
5. Comprueba que el volumen oculto abarque el cuerpo y siga bloqueando `Visibility`: pala y disparos necesitan poder impactar al actor.

Importa un modelo con esqueleto y sus animaciones compatibles para usar Skeletal Mesh. Un GLB o FBX estático no obtiene huesos o animaciones por sí solo. Si las animaciones usan otro esqueleto, tendrás que retargetearlas o conseguir animaciones del mismo rig.

## Animaciones del zombi

Asigna un Animation Blueprint al componente Skeletal Mesh. Desde su propietario puedes consultar `Health` e `IsGrabbingPlayer` para seleccionar estados. Los eventos **OnGrabStarted** y **OnGrabEnded** del zombi permiten reproducir y detener la animación de agarre. El jugador también expone eventos con esos nombres para efectos de cámara o sonido.

Los zombis actuales son `AActor` que recorren puntos mediante `SetActorLocation`: no son `ACharacter`, no usan NavMesh ni root motion. Para decidir entre caminar y quedarse quietos, mide la distancia entre posiciones sucesivas dividida por Delta Seconds; no dependas de `GetVelocity`, que puede permanecer a cero con este movimiento. Usa animaciones *in-place*, sin desplazamiento de raíz.

Para sincronizar mordiscos con la animación, propondría añadir un evento `OnAttackPerformed` en el momento del ataque o trasladar el daño a un Anim Notify. Ese evento no existe todavía: la base actual aplica el daño directamente por intervalo. No dupliques el daño entre el intervalo y el notify. Del mismo modo, la muerte destruye inmediatamente el actor; una animación de muerte requerirá separar «marcar muerto» y «destruir al terminar» sin duplicar la recompensa.

## Programación del zombi

Sobrescribe **HabilidadEspecial(DeltaSeconds)** en Blueprint para una habilidad propia; conserva la llamada al padre cuando sea necesaria. Este evento se ejecuta durante la lógica de ruta, no mientras el zombi sujeta al jugador ni después de terminar la partida. Por ejemplo, puedes acumular un temporizador y cambiar `Speed` para una aceleración temporal.

Para modificar por completo la IA, crea una subclase C++ y sobrescribe `Tick`, decidiendo qué comportamiento del padre mantener. No añadas un segundo sistema de movimiento que compita con el recorrido por carriles. Si cambias el actor base a `ACharacter`, tendrás que adaptar el director y las búsquedas de blancos, que actualmente trabajan con `AZombie`.

## Tu primera planta

1. Crea un Blueprint hijo de **PlantaOfensiva**, **PlantaDefensiva** o **PlantaMiscelanea**. Puedes partir de **PlantaGirasol** si deseas otro productor de sol.
2. Ajusta `MaxHealth`, `WaterDrain` y `ActionInterval`; en ofensivas, `AttackDamage` y `AttackRange`; en girasoles, `Production`.
3. Incorpora tu modelo usando la separación visual/colisión descrita antes.
4. Crea **Miscellaneous → Data Asset → GardenSeed**, por ejemplo `DA_Semilla_LanzabroteAzul`. Configura `DisplayName`, `PlantClass`, `Cost`, `Cooldown`, `Aquatic` y sus multiplicadores.
5. En tu Blueprint del director, asigna ese Data Asset a una de las ocho posiciones de `SeedLoadout`. Las posiciones vacías conservan las semillas de prueba. Una semilla acuática solo puede plantarse en piscina.

## Programación y animación de plantas

- **Ofensiva:** sobrescribe `Atacar` para cambiar el ataque. Si llamas al padre conservas su disparo instantáneo, búsqueda por carril y prioridad de rescate. No apliques otro daño adicional accidentalmente.
- **Miscelánea:** sobrescribe `EjecutarEspecial`; el ciclo de hidratación invoca ese evento con la cadencia efectiva. Es el punto para un productor, un imán o un potenciador.
- **Defensiva:** la base es una barrera de vida alta; no tiene ataque automático.
- **Animación:** usa un Skeletal Mesh y Animation Blueprint para reposo/sequía, consultando `Hydration` y `GetRendimiento`. Para disparos sincronizados conviene introducir un evento al confirmar el blanco, en vez de animar cada intento fallido de `Atacar`.

Si implementas proyectiles, al impactar usa **Apply Damage** contra el zombi y pasa **la planta que lo disparó como Damage Causer**. Así conserva la liberación del jugador. Si pasas el proyectil como causante, el sistema actual no lo reconocerá como ayuda de una planta. Conserva la referencia de la planta mientras el proyectil está activo o amplía el contrato de rescate si quieres que sobreviva a la destrucción del emisor.

## Reglas del agarre implementado

Contacto a un máximo de 135 cm entre centros por defecto, diferencia de altura máxima de 90 cm y línea de visión libre. No hay persecución nueva: los zombis siguen sus rutas y agarran si te acercas. Cada jugador solo puede tener un captor. Durante el agarre puede mirar, pero no caminar, saltar, plantar, usar herramientas o comprar mejoras.

Dispone de **3 segundos**. Un impacto con daño positivo de una planta sobre su captor lo libera sin exigir matar al zombi. También se libera si el captor muere o desaparece. Después tiene **2 segundos** de protección frente a un nuevo agarre. Sin rescate, termina la partida y Enter reinicia. Las plantas respetan alcance, carril, sector y obstáculos del escenario; plantas aliadas y jugador no bloquean sus disparos.

El agarre tiene lógica y aviso de HUD; todavía no incluye una animación corporal específica. Los eventos expuestos permiten añadirla al incorporar tus modelos.
