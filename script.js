const CHANNEL_ID = "3428611";

const API =
`https://api.thingspeak.com/channels/${CHANNEL_ID}/feeds.json?results=20`;


let soilData=[];
let tempData=[];
let timeData=[];
let history=[];



let soilChart =
new Chart(
document.getElementById("soilChart"),
{

type:"line",

data:{

labels:timeData,

datasets:[{

label:"Soil Moisture (%)",

data:soilData,

}]

}

});



let tempChart =
new Chart(
document.getElementById("tempChart"),
{

type:"line",

data:{

labels:timeData,

datasets:[{

label:"Temperature (°C)",

data:tempData,

}]

}

});





async function loadData(){

let response =
await fetch(API);


let data =
await response.json();



let feeds =
data.feeds;



soilData=[];
tempData=[];
timeData=[];
history=[];



feeds.forEach(item=>{


let date = new Date(item.created_at);

let time =
date.toLocaleDateString("id-ID",{
    day:"2-digit",
    month:"short",
    year:"numeric"
})
+"\n"+
date.toLocaleTimeString("id-ID",{
    hour:"2-digit",
    minute:"2-digit"
});



let soil =
Number(item.field1);



let temp =
Number(item.field2);



let pump =
Number(item.field3)==1?
"ON":
"OFF";



timeData.push(time);

soilData.push(soil);

tempData.push(temp);



history.push({

time,
soil,
temp,
pump

});


});



let last =
history[history.length-1];



document.getElementById("soil")
.innerHTML =
last.soil+" %";



document.getElementById("temperature")
.innerHTML =
last.temp+" °C";



document.getElementById("pump")
.innerHTML =
last.pump;



soilChart.data.labels=timeData;
soilChart.data.datasets[0].data=soilData;
soilChart.update();



tempChart.data.labels=timeData;
tempChart.data.datasets[0].data=tempData;
tempChart.update();




let table="";


history.reverse()
.forEach(row=>{


table+=`

<tr>

<td>${row.time}</td>

<td>${row.soil}</td>

<td>${row.temp}</td>

<td>${row.pump}</td>

</tr>

`;


});


document.getElementById("dataTable")
.innerHTML=table;


}


function downloadCSV(){


let csv =
"Time,Soil,Temperature,Pump\n";



history.forEach(row=>{


csv +=
`${row.time},${row.soil},${row.temp},${row.pump}\n`;


});



let blob =
new Blob(
[csv],
{type:"text/csv"}
);



let url =
URL.createObjectURL(blob);



let a =
document.createElement("a");

a.href=url;

a.download="irrigation_data.csv";

a.click();


}



loadData();


// refresh setiap 16 detik
setInterval(loadData,16000);
