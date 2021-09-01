// const url = "http://sheltered-dawn-53976.herokuapp.com/api";
const url = "http://localhost:3001/api";

const maxBlue = 12500;
const maxRed = 2500;

let unsaved = false;
let sensors;

const timeOffsetInHours = -(new Date()).getTimezoneOffset()/60;

const apalah = () => {
    if (sensors) {
        const labels = sensors[0].values.map( (elm, i) => {
            const rawString = new Date(new Date(elm.date).getTime()).toString();
            const temp = rawString.split(" GMT");
            return temp[0];
            // return new Date(elm.date)
        })
        const rData = sensors[0].values.map( (elm) => {
            return elm.r;
        })
        const bData = sensors[0].values.map( (elm) => {
            return elm.b;
        })

        bikin(labels, rData, bData);
    }
}

$(function() {
    console.log( "ready!" );
    

    $.ajax({
        method: "GET",
        url: url + "/output/1",
        success: [data => {
            console.log(data)
            $("#red").val(data.values.r/maxRed*100)
            $("#blue").val(data.values.b/maxBlue*100)
            $("#begin").val(data.time.begin)
            $("#end").val(data.time.end)

            $("#redText").text(data.values.r/maxRed*100 + "%")
            $("#blueText").text(data.values.b/maxBlue*100 + "%")
        }]
      });
    
    const enableButton = () => $( "button" ).prop( "disabled", false );
    const disableButton = () => $( "button" ).prop( "disabled", true );

    disableButton();

    $("#begin").change( enableButton );
    $("#end").change( enableButton );
    $("#red").change( () => {
        enableButton();
        $("#redText").text($("#red").val() + '%');
    });
    $("#blue").change( () => {
        enableButton();
        $("#blueText").text($("#blue").val() + '%');
    } );

    $("button").click( () => {
        console.log("clicked");
        const setLed = {
            r: Number($("#red").val())/100*maxRed,
            b: Number($("#blue").val())/100*maxBlue,
            begin: $("#begin").val(),
            end: $("#end").val()
        }
        console.log(setLed)

        $.ajax({
            method: "POST",
            url: url + "/led/1",
            contentType: "application/json",
            data: JSON.stringify(setLed),
            success: [data=>console.log(data) ,disableButton]
          });
    })
    // get sensors


    
});