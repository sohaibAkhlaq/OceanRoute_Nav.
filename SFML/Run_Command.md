# Run SFML Project (Single Command)

## Step-by-step
g++ main.cpp -I SFML/include -L SFML/lib -o app.exe -lsfml-graphics -lsfml-window -lsfml-system
$env:PATH = "$PWD\SFML\lib;" + $env:PATH
./app.exe

## One-line command
g++ main.cpp -I SFML/include -L SFML/lib -o app.exe -lsfml-graphics -lsfml-window -lsfml-system; $env:PATH = "$PWD\SFML\lib;" + $env:PATH; ./app.exe

# Current
g++ SFML.cpp -I SFML/include -L SFML/lib -o app.exe -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lsfml-network; if ($?) { $env:PATH="$PWD\SFML\lib;"+$env:PATH; ./app.exe; rm app.exe } else { echo "❌ Build failed" }


g++ SFML.cpp -I SFML/include -L SFML/lib -o app.exe -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lsfml-network;$env:PATH = "$PWD\SFML\lib;" + $env:PATH; ./app.exe ; rm app.exe

g++ Adjust.cpp -I SFML/include -L SFML/lib -o app2.exe -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lsfml-network;$env:PATH = "$PWD\SFML\lib;" + $env:PATH; ./app2.exe