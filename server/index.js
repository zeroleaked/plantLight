const express = require('express')
const app = express()
const cors = require('cors')

app.use(express.static('static'));
app.use(cors());
app.use(express.json());

const requestLogger = (request, response, next) => {
    console.log('Method:', request.method)
    console.log('Path:  ', request.path)
    console.log('Body:  ', request.body)
    console.log('---')
    next()
  }
  
  app.use(requestLogger)

let sensors = [
    {
        id: 1,
        values: []
    }
  ]

const maxBlue = 12500;
const maxRed = 2500;

let leds = [
    {
        id: 1,
        time: {
          begin: "10:00",
          end: "18:00"
        },
        values: {
                r: 50/100*maxRed,
                b: 50/100*maxBlue
            }
    }
]
  
for (hour = 24 * 3; hour >= 0; hour--) {
    const data = {}
    
    data.date = new Date(new Date().getTime() - 1000*60*60*hour);
    data.r = Math.round(Math.random() * 0);
    data.b = Math.round(Math.random() * 0);
    sensors[0].values.push(data)
}

app.get('/api/input', (request, response) => {
  response.json(sensors)
})

app.get('/api/output/:id', (request, response) => {
    const id = Number(request.params.id);
    const led = leds.find( led => led.id === id);
    response.json(led);
  })

app.post('/api/input', (request, response) => {
    const newInLight = request.body;
    newInLight.date = new Date();
    sensors = sensors.map( sensor => {
        if (sensor.id === newInLight.id) {
            sensor.values = sensor.values.filter( (elm, i) => i !== 0);

            sensor.values = sensor.values.concat({
                ...newInLight.values,
                date: new Date()
            });

        }
        return sensor;
    })
    response.json(newInLight);
  }
)

app.post('/api/led/:id', (request, response) => {
  const id = Number(request.params.id);
  const newLed = {
    id,
    time: {
      begin: request.body.begin,
      end: request.body.end
    },
    values: {
      r: request.body.r,
      b: request.body.b
    }
  };
  leds = leds.map( led => {
    if (led.id === id) {
      return newLed;
    }
    else return led;
  })
  response.json(newLed);
})

const PORT = process.env.PORT || 3001
app.listen(PORT, () => {
  console.log(`Server running on port ${PORT}`)
})