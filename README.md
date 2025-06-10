# Pufu OS

Un sistema operativo minimalista y elegante escrito en C.

## Características

- Sistema de nodos para cargar y ejecutar código dinámicamente
- Soporte para múltiples arquitecturas (ARM, x86_64)
- Hot-reload para desarrollo en tiempo real
- Interfaz de línea de comandos simple y efectiva

## Requisitos

- GCC
- Make
- Sistema operativo Linux

## Compilación

```bash
make clean
make
```

## Ejecución

```bash
make run
```

## Estructura del Proyecto

- `src/`: Código fuente
  - `core/`: Núcleo del sistema
  - `sockets/`: Implementaciones específicas por arquitectura
- `include/`: Archivos de cabecera
- `bin/`: Binarios compilados
- `obj/`: Archivos objeto

## Licencia

MIT 