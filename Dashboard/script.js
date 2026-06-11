console.log("Smart Home Energy Saver Dashboard Loaded");

function toggleTheme(){
    document.body.classList.toggle("dark-mode");
}

function downloadReport(){

    const report = `
Smart Home Energy Saver Report

Motion Status: ${document.getElementById("motion").innerText}

Light Status: ${document.getElementById("light").innerText}

Room Occupancy: ${document.getElementById("occupancy").innerText}

Energy Alert: ${document.getElementById("alert").innerText}

Generated Successfully
`;

    const blob = new Blob([report], {type:"text/plain"});

    const a = document.createElement("a");
    a.href = URL.createObjectURL(blob);
    a.download = "Energy_Report.txt";
    a.click();
}

setInterval(() => {

    document.getElementById("clock").innerText =
        new Date().toLocaleTimeString();

},1000);

setInterval(() => {

    const motion = document.getElementById("motion");
    const light = document.getElementById("light");
    const occupancy = document.getElementById("occupancy");
    const activity = document.getElementById("activity");

    if(Math.random() > 0.5){

        motion.innerText = "Motion Detected";
        light.innerText = "ON";
        occupancy.innerText = "Occupied";
        activity.innerText = "Person Detected";

    }
    else{

        motion.innerText = "No Motion Detected";
        light.innerText = "OFF";
        occupancy.innerText = "Empty";
        activity.innerText = "No Activity";

    }

},3000);