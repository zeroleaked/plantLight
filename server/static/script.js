// const url = "http://sheltered-dawn-53976.herokuapp.com/api";
const url = "http://localhost:3001/api";

const maxBlue = 12500;
const maxRed = 2500;

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
            return Math.round(elm.r/maxRed*100);
        })
        const bData = sensors[0].values.map( (elm) => {
            return Math.round(elm.b/maxBlue*100);
        })

        bikin(labels, rData, bData);
    }
}

$(function() {
    console.log( "ready!" );


    // get sensors
    $.ajax({
        url: url + "/input",
        success: [data => {
            console.log(data);
            sensors = data;
        }, apalah]
      });


    
});

const bikin = (labels, rData, bData) => {
    console.log(rData)
    console.log(bData)

    var ctx = document.getElementById('myChart').getContext('2d');
    var myChart = new Chart(ctx, {
        type: 'bar',
        data: {
            labels,
            datasets: [
                {
                    label: 'red light',
                    data: rData,
                    backgroundColor: 'red',
                },
                {
                    label: 'blue light',
                    data: bData,
                    backgroundColor: 'blue'
                },
            ]
        },
        options: {
            scales: {
                x: {
                    ticks: {
                        maxTicksLimit: 10
                    }
                },
                y: {
                    beginAtZero: true,
                    ticks: {
                        callback: function(value, index, values) {
                            return value + '%';
                            // return ((value / 800) * 100) + '%';
                        }
                    }
                },
            },
             maintainAspectRatio: false,
        }
    });
}