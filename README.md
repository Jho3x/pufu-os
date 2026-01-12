# Pufu OS (v0.7 'Lumina')

**Pufu OS** es un sistema operativo experimental. Se basa en mis ideas y vibecoding que use porque no se programar en C (igual lo estoy estudiando, no se enojen jaja).
Solamente es una herramienta pequeña y sencilla para poder usar un motor 3d y hacer calculos de AI sin necesidad de un ordenador potente.

**"¿Como se usa? (🚀 Quick Start)"**

Para usar Pufu se puede usar o bien un google colab o bien la descarga local.
Paso a decir los pasos para ambos y mas abajo voy a explicar los detalles de Pufu.

**"Pasos para Google Colab"**

1. Para usar Pufu en Google Colab, necesitas una cuenta en ngrok (https://dashboard.ngrok.com/get-started/your-authtoken) ese enlace lleva al autotoken. Cuando tengas una cuenta vas a la barra izquierda donde dice "getting started" -> "your authtoken" -> "copy authtoken".
El autotoken es una contraseña segura que te da ngrok para que puedas usarlo en el colab y ver la pantalla de Pufu OS.

2. Despues vas a ejecutar estos comandos en el colab:
```bash
# esto es por si ya tienes un pufu-os en el colab y queres borrarlo para volver a compilar
!rm -rf pufu-os
print("Directorio 'pufu-os' eliminado.")
```
```bash
# clonamos el repositorio
!git clone https://github.com/Jho3x/pufu-os.git
print("Repositorio clonado.")
```
```bash
# cambiamos al directorio
!cd pufu-os
print("Directorio cambiado.")
```
```bash
# ejecutamos el script
python3 tools/pufu_colab.py
```

Esto va a hacer que se cargue una pantalla masomenos asi:

```=== Ngrok Configuration ===
Ngrok requires a free Authtoken to be reliable.
Get one here: https://dashboard.ngrok.com/get-started/your-authtoken
Enter your Ngrok Authtoken: ****************************** (ACA VA EL AUTOTOKEN DE NGROK)
Installing dependencies...
Cleaning up old processes...
Compiling Pufu OS...
Compilation Success.
Configuring for Cloud (GUI Mode)...
Launching Pufu OS (Background)...

=== Pufu Bootloader [v0.7 Lumina] ===
Iniciando carga dinámica del socket...
[System] Watchdog initialized.
[Loader] Cargando socket dinámico: bin/drivers/socket_arm.so
Inicializando socket ARM...
[Loader] Socket cargado exitosamente.
VirtualBus (IPC) Initialized.
pufu_so node parser initialized
.... (resto de comandos)


                              x
    ▓         ▓               0
    ▓ ▓     ▓ ▓               1
  ▓ ▓ ▓ ▓ ▓ ▓ ▓ ▓     ▓ ▓ ▓   2
  ▓ ▓ ▓ ▓ ▓ ▓ ▓ ▓   ▓ ▓ ▓ ▓ ▓ 3
  ▓     ▓ ▓     ▓   ▓ ▓   ▓ ▓ 4
  ▓     ▓ ▓     ▓   ▓ ▓       5
  ▓ ▓ ▓ ▓ ▓ ▓ ▓ ▓   ▓ ▓ ▓ ▓   6
  ▓ ▓ ▓ ▓ ▓ ▓ ▓ ▓     ▓ ▓ ▓   7
        ▓ ▓             ▓ ▓   8
      ▓ ▓ ▓ ▓ ▓ ▓ ▓ ▓ ▓ ▓ ▓   9
      ▓ ▓ ▓ ▓ ▓ ▓ ▓ ▓ ▓ ▓     A
      ▓     ▓   ▓ ▓   ▓ ▓     B
      ▓     ▓   ▓ ▓   ▓ ▓     C
      ▓     ▓     ▓     ▓     D
                              E
            PUFU!             F
Pardum Felidum Operating System

=== Bienvenido a Pufu OS ===

=== SESSION MANAGER ===
User:
ACA SE QUEDA ESPERANDO QUE INGRESES EL USUARIO
Password:
ACA SE QUEDA ESPERANDO QUE INGRESES LA CONTRASEÑA
```

Una vez que ingreses el usuario y la contraseña, aparece algo asi:

```bash
Access Granted.
Welcome, ******
DEBUG: Reading user configuration...
... (Otros comandos de carga)
========================================
PUFU OS PUBLIC CLOUD ACCESS (NGROK)
========================================
Click here to access Pufu OS: https:// enlace a la pagina de la gui ejecutandose en el colab
========================================
... (cada cierta cantidad de segundos aca se van a imprimir mensajes de status)
```

Para cerrar Pufu OS en el colab, solo tienes que ejecutar el siguiente comando:

```bash
python3 tools/pufu_colab.py --shutdown
```

Aunque lo mas normal es simplemente cerrar la ventana de gui en el navegador, para luego detener el colab desde su icono de stop.

Cuando Pufu es terminado aparece algo asi:

```bash
[Input Loop Stopped] Pufu OS continues in background...

=== Shutting down Pufu ===
[Bootloader] Cleaning up Node System...
[TRINITY] Shutting down. Freeing payloads...
[TRINITY] Shutting down. Freeing payloads...

[Kernel] FATAL ERROR: Signal 11 caught! # estos errores son normales, solo dicen que no se cerro correctamente el kernel
[Kernel] Dumping Crash Log to 'crash.log'...
```


**"Pasos para ejecutar Pufu OS localmente"**

1. localmente se tiene que descargar el repositorio como .zip y descomprimirlo.

2. una vez descomprimido se tiene que ejecutar el siguiente comando por terminal en la carpeta pufu-os:

```bash
make run
```


## 📚 Documentation

Detailed specifications for the custom languages and protocols:

### 1. Languages & Protocols
*   **[Paw (ASM)](docs/specs/paw_dictionary.md)**: The Assembly language for the Pufu VM. Defines the instruction set.
*   **[Meow (UI)](docs/specs/meow_dictionary.md)**: The Markup Language for defining the Fractal UI (Trinity).
*   **[Claw (Kernel)](docs/specs/claw_dictionary.md)**: The Interface Description Language for Kernel Syscalls and IPC.

### 2. Guides
*   **[Cloud Deployment](docs/CLOUD_DEPLOY.md)**: How to run Pufu OS in Google Colab (Headless/Web Mode).

## 🛠️ Prerequisites

To build and run Pufu OS, you need a standard Linux environment:
*   `gcc` (Build Essential)
*   `make`
*   `python3` (For build tools and launcher)


