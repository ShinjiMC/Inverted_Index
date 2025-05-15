import os
import sys
import unicodedata
import re

def clean_text(text):
    # Elimina acentos y normaliza caracteres especiales
    text = unicodedata.normalize('NFD', text).encode('ascii', 'ignore').decode('utf-8')
    # Opcional: elimina caracteres no alfanuméricos y signos raros (excepto ., espacio y \n)
    text = re.sub(r"[^a-zA-Z0-9\s\.,]", "", text)
    return text

# 100 palabras comunes (frecuentes en español) – ya sin tildes
COMMON_WORDS = [
    "inteligencia", "artificial", "algoritmo", "modelo", "datos", "informacion", "aprendizaje", "automatico", 
    "sistema", "tecnologia", "proceso", "computadora", "prediccion", "analisis", "representacion", "entrada",
    "salida", "clasificacion", "regresion", "cluster", "conjunto", "parametro", "variable", "entrenamiento",
    "prueba", "optimizacion", "funcion", "objetivo", "error", "evaluacion", "validacion", "generalizacion",
    "convergencia", "ajuste", "pesos", "neuronas", "red", "capa", "filtro", "caracteristicas", "texto", "imagen",
    "voz", "traduccion", "reconocimiento", "patrones", "estimacion", "proceso", "complejidad", "tiempo", "memoria",
    "reduccion", "dimension", "hipotesis", "predicciones", "conjunto", "precision", "recall", "f1", "metrica",
    "probabilidad", "inferencia", "distribucion", "normal", "gaussiana", "lineal", "no_lineal", "entropia", 
    "mutua", "gradiente", "descenso", "backpropagation", "regularizacion", "l1", "l2", "overfitting", 
    "underfitting", "modelo", "ensemble", "bosque", "aleatorio", "boosting", "bagging", "kmeans", "svm", 
    "naive", "bayes", "transformada", "fourier", "pca", "tsne", "token", "frecuencia", "tfidf", "distancia",
    "coseno", "euclidea", "manhattan", "hamming"
]

# Palabras únicas por archivo, ya sin tildes
EXTRA_WORDS = [
    ["vision", "imagen", "segmentacion", "deteccion", "rostro", "objeto", "borde", "filtro", "color", "forma",
     "contraste", "umbral", "patron", "ruido", "resolucion", "matriz", "pixeles", "entrenado", "camara", "video",
     "flujo", "mapa", "calor", "histograma", "saturacion", "brillo", "profundidad", "esquinas", "gradiente", "contorno",
     "movimiento", "frame", "clip", "fps", "reconstruccion", "escalar", "zoom", "rotacion", "transformacion"],
    ["texto", "documento", "palabra", "frecuencia", "ngrama", "simbolo", "caracter", "lenguaje", "gramatica", "sintaxis",
     "semantica", "vector", "significado", "lectura", "escritura", "traduccion", "resumen", "noticia", "revision", "dialogo",
     "chatbot", "respuestas", "pregunta", "titulo", "contenido", "expresion", "literal", "parafrasis", "resena", "parrafo",
     "tema", "categoria", "indice", "corpus", "vocabulario", "frase", "segmento", "subtitulo", "encabezado"],
    ["sonido", "onda", "frecuencia", "audio", "melodia", "voz", "habla", "microfono", "grabacion", "tono", 
     "ritmo", "velocidad", "reconocimiento", "sintesis", "ruido", "formante", "fonema", "acustica", "pitch", "armonia",
     "compresion", "mezcla", "ecualizador", "banda", "filtro", "senal", "amplitud", "sampleo", "digital", "analogica",
     "modulacion", "distorsion", "canal", "bitrate", "buffer", "paquete", "latencia", "auricular", "streaming"],
    ["robot", "sensor", "actuador", "movimiento", "trayectoria", "control", "bateria", "motor", "rueda", "direccion",
     "velocidad", "giroscopio", "entorno", "navegacion", "mapa", "localizacion", "obstaculo", "evitacion", "camara", "vision",
     "profundidad", "servo", "pinza", "brazo", "humanoide", "movil", "deteccion", "colision", "algoritmo", "busqueda",
     "trayectoria", "comando", "sensorial", "resolucion", "feedback", "posicion", "coordenada", "angulo", "robotica"],
    ["biologia", "genoma", "celula", "proteina", "adn", "arn", "enzima", "mutacion", "estructura", "funcion",
     "organismo", "microbio", "bacteria", "virus", "sistema", "inmune", "respuesta", "sintoma", "vacuna", "infeccion",
     "anticuerpo", "diagnostico", "prueba", "terapia", "tratamiento", "medicina", "hospital", "salud", "genetico", "herencia",
     "biomarcador", "patologia", "muestra", "biopsia", "epidemiologia", "contagio", "epidemia", "genetico", "bioinformatica"]
]

def generate_file(filename, common_words, extra_words, target_size_gb):
    all_words = common_words + extra_words
    sentence = " ".join(all_words)
    paragraph = f"En este documento se abordan temas como {sentence}.\n" * 5
    paragraph_clean = clean_text(paragraph)
    block = (paragraph_clean * 10).encode('utf-8')

    block_size = len(block)
    target_size_bytes = target_size_gb * 1024**3
    written = 0

    with open(filename, 'wb') as f:
        while written < target_size_bytes:
            f.write(block)
            written += block_size
            if written % (1024**3) < block_size:
                print(f'📦 {filename}: {written / 1024**3:.2f} GB escritos')

    print(f'✅ Archivo \"{filename}\" generado ({written / 1024**3:.2f} GB)')

if __name__ == "__main__":
    try:
        gb = int(sys.argv[1]) if len(sys.argv) > 1 else 4
    except ValueError:
        print("❌ El argumento debe ser un número entero (GB)")
        sys.exit(1)

    print(f"📚 Generando 5 archivos de {gb} GB sin tildes ni caracteres especiales...\n")
    for i in range(5):
        filename = f"text_clean_{i+1}_{gb}GB.txt"
        generate_file(filename, COMMON_WORDS, EXTRA_WORDS[i], gb)

    total_words = set(COMMON_WORDS)
    for block in EXTRA_WORDS:
        total_words.update(block)
    print(f"\n🔠 Palabras unicas totales: {len(total_words)}")
