from flask import Flask, render_template_string
from datetime import datetime
from collections import Counter

app = Flask(__name__)

VENTAS_PATH = "/home/Pandax/Documents/VendigMachineLogs/ventas.txt"

HTML_TEMPLATE = """
<!DOCTYPE html>
<html>
<head>
    <title>Dashboard de Ventas</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
</head>
<body>
    <h1>Dashboard de Ventas</h1>
    <p>Total de ventas: {{ total }}</p>
    <p>Garrafones: {{ Garrafon }}, Galones: {{ Galon }}</p>

    <canvas id="ventasChart" width="400" height="200"></canvas>
    <script>
        const ctx = document.getElementById('ventasChart').getContext('2d');
        const chart = new Chart(ctx, {
            type: 'bar',
            data: {
                labels: {{ labels|safe }},
                datasets: [{
                    label: 'Ventas por día',
                    data: {{ data|safe }},
                    borderWidth: 1
                }]
            },
            options: {
                scales: {
                    y: {
                        beginAtZero: true
                    }
                }
            }
        });
    </script>
</body>
</html>
"""

@app.route("/")
def dashboard():
    try:
        with open(VENTAS_PATH, "r") as f:
            lineas = f.readlines()
    except FileNotFoundError:
        lineas = []

    total = len(lineas)
    tipos = ["Garrafon" if "Garrafon" in l else "Galon" for l in lineas]
    conteo_tipos = Counter(tipos)

    # Agrupar ventas por fecha (YYYY-MM-DD)
    fechas = [l.split(" - ")[0].split()[0] for l in lineas if " - " in l]
    conteo_fechas = Counter(fechas)

    return render_template_string(
        HTML_TEMPLATE,
        total=total,
        Garrafon=conteo_tipos.get("Garrafon", 0),
        Galon=conteo_tipos.get("Galon", 0),
        labels=list(conteo_fechas.keys()),
        data=list(conteo_fechas.values())
    )

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)
