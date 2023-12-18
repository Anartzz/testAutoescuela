package main

import (
	"fmt"
	"io/ioutil"
	"net/http"
	"strings"
)

func mandarArchivo(w http.ResponseWriter, r *http.Request) {
	uri := "../app/" + r.RequestURI
	content, err := ioutil.ReadFile(uri)
	if err != nil {
		http.Error(w, "Error al leer el archivo JavaScript", http.StatusInternalServerError)
		return
	}
	var contentType string
	if strings.HasSuffix(uri, ".js") {
		contentType = "application/javascript"
	} else if strings.HasSuffix(uri, ".css") {
		contentType = "text/css"
	} 

	if contentType != "" {
		w.Header().Set("Content-Type", contentType)
	}
	_, err = w.Write(content)
	if err != nil {
		http.Error(w, "Error al escribir el archivo", http.StatusInternalServerError)
		return
	}
}

func main() {
	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		http.ServeFile(w, r, "../app/index.html")
	})
	http.HandleFunc("/script.js", mandarArchivo)
	http.HandleFunc("/styles.css", mandarArchivo)
    http.HandleFunc("/testColeccion", func(w http.ResponseWriter, r *http.Request) {
        //http.ServeFile(w, r, "..")
    })

	fmt.Println("Servidor en ejecución en http://localhost:8080")
	http.ListenAndServe(":8080", nil)
}


