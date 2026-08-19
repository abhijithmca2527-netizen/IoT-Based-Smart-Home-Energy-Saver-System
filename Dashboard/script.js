console.log("Smart Home Energy Saver Dashboard Loaded");


// ======================================
// DARK MODE
// ======================================

function toggleTheme() {

    document.body.classList.toggle("dark-mode");

    const themeButton =
        document.getElementById("themeButton");


    if (document.body.classList.contains("dark-mode")) {

        themeButton.innerText =
            "☀️ Light Mode";

        localStorage.setItem(
            "dashboardTheme",
            "dark"
        );

    } else {

        themeButton.innerText =
            "🌙 Dark Mode";

        localStorage.setItem(
            "dashboardTheme",
            "light"
        );

    }
}


// ======================================
// LOAD SAVED THEME
// ======================================

window.addEventListener("DOMContentLoaded", () => {

    const savedTheme =
        localStorage.getItem("dashboardTheme");


    if (savedTheme === "dark") {

        document.body.classList.add("dark-mode");

        document.getElementById("themeButton").innerText =
            "☀️ Light Mode";

    }

});


// ======================================
// DOWNLOAD REPORT
// ======================================

function downloadReport() {

    const report = `
IoT-Based Smart Home Energy Saver System
=========================================

Motion Status:
${document.getElementById("motion").innerText}

Light Status:
${document.getElementById("light").innerText}

Room Occupancy:
${document.getElementById("occupancy").innerText}

Activity:
${document.getElementById("activity").innerText}

Energy Used:
${document.getElementById("energyUsed").innerText}

Energy Saved:
${document.getElementById("energySaved").innerText}

=========================================

Generated On:
${new Date().toLocaleString()}
`;


    const blob = new Blob(
        [report],
        {
            type: "text/plain"
        }
    );


    const url =
        URL.createObjectURL(blob);


    const link =
        document.createElement("a");


    link.href = url;

    link.download =
        "Smart_Home_Energy_Report.txt";


    document.body.appendChild(link);

    link.click();

    document.body.removeChild(link);

    URL.revokeObjectURL(url);
}