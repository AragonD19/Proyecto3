# Proyecto 3: Raytracing

Este es un programa en C++ que utiliza la biblioteca Simple DirectMedia Layer (SDL) para crear un sencillo trazador de rayos. El trazador de rayos es capaz de renderizar escenas con objetos básicos como cubos y esferas, e implementa iluminación, sombras y reflexión.

## Dependencias

- SDL: Simple DirectMedia Layer se utiliza para crear la ventana, manejar eventos y renderizar.

## Resumen del Código

### Componentes Principales

- **`main.cpp`**: Punto de entrada principal del programa. Inicializa SDL, configura la escena y maneja el bucle principal de renderizado.

- **`object.h`**: Una clase base para diferentes tipos de objetos en la escena. Tiene un método virtual `rayIntersect` para verificar intersecciones con rayos.

- **`sphere.h`** y **`cube.h`**: Clases que representan esferas y cubos, ambas derivadas de la clase base `Object`. Implementan el método `rayIntersect` específico para sus formas.

- **`light.h`**: Representa una fuente de luz puntual en la escena. Tiene propiedades como posición, intensidad y color.

- **`camera.h`**: Define la cámara utilizada para el renderizado, incluyendo su posición, orientación y campo de visión.

- **`skybox.h`**: Maneja el skybox, proporcionando colores basados en la dirección del rayo.

### Funciones de Trazado de Rayos

- **`castShadow`**: Proyecta sombras desde objetos para determinar si un punto está en sombra o iluminado por la fuente de luz.

- **`castRay`**: Rastrea un rayo en la escena, verifica intersecciones con objetos, calcula la iluminación y maneja reflexiones y refracciones.

### Configuración

- **`setUp`**: Inicializa materiales y configura la escena con varios cubos, esferas, luces y materiales.

### Renderizado

- **`point`**: Dibuja un punto en la pantalla utilizando SDL.

### Materiales

- **`Material`**: Define las propiedades de un material, incluyendo color difuso, coeficiente especular, reflectividad, transparencia y más.

## Configuración de la Escena

La función `setUp` inicializa materiales para diferentes objetos y llena el vector `objects` con cubos y esferas dispuestos para formar una escena. Se utilizan diferentes materiales para diversos efectos visuales.

## Trazado de Rayos

El núcleo del trazador de rayos está en la función `castRay`, que traza rayos de manera recursiva en la escena, calcula la iluminación y maneja la reflexión y refracción.

## Renderizado

El bucle principal de renderizado en `main.cpp` llama a la función `castRay` para cada píxel, y los colores resultantes se muestran en la ventana utilizando SDL.
