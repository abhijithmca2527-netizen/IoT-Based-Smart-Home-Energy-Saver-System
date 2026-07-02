function downloadReport() {

    const report = `
Smart Home Energy Saver Report

=====================================

Motion Status : ${document.getElementById("motion").innerText}

Light Status : ${document.getElementById("light").innerText}

Room Occupancy : ${document.getElementById("occupancy").innerText}

Activity : ${document.getElementById("activity").innerText}

Energy Used : ${document.getElementById("energyUsed").innerText}

Energy Saved : ${document.getElementById("energySaved").innerText}

=====================================

Generated Successfully
`;

    const blob = new Blob([report], { type: "text/plain" });

    const a = document.createElement("a");

    a.href = URL.createObjectURL(blob);

    a.download = "Energy_Report.txt";

    a.click();
}