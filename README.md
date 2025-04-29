# EmbarcaTechProjetoRevisão
Projeto de desenvolvimento de um ohmímetro na BitDogLab. Atividade proposta no programa de capacitação EmbarcaTech - TIC 37 - Fase 2

# Vídeo Demonstração

https://youtu.be/3FsRxsFM3Ao

# Hardware/Firmware

Projeto desenvolvido em uma placa de desenvolvimento BitDogLab, versão 6.3.<br>
Desenvolvimento de firmware feito através do PicoSDK, com a IDE Visual Studio Code.

# Instruções

O programa obtém o valor da resistência, em ohms, de um resistor baseada na tensão que está alocada nele.<br>
Com dois resistores em série e, conhecendo o valor da resistência do outro, é possível determinar a tensão alocada.<br>
Este valor, inclusive, pode ser alterado nas macros do programa em caso de troca do referencial.<br><br>
Através da leitura do ADC, o valor obtido é convertido em ohms e seu correspondente na escala e24 é encontrado.<br>
Com isso, é exibido o código de cores na matriz de LEDs, considerando um resistor de 4 faixas. Além disso, as cores são exibidas no display por extenso, em conjunto com a resistência obtida.