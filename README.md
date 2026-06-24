# Trabalho 1 - Redes de Computadores I

Feito por:

> André Akira A Abe Aracema (GRR20240191)

> Andrieli Luci Gonçalves (GRR20244903)

## 1 - Compilar

```
make
```

## 2 - Executar

O servidor e o cliente devem rodar em máquinas diferentes, conectadas diretamente por cabo de rede. Ambos precisam de privilégios de root.

**Servidor:**
```bash
sudo ./pacman -s
```

**Cliente:**

```bash
sudo ./pacman -c
```

## Controles

| Tecla | Ação |
|-------|------|
| `w` | Mover para cima |
| `s` | Mover para baixo |
| `a` | Mover para esquerda |
| `d` | Mover para direita |

## Log (somente no servidor)

```bash
tail -f log.txt
```

## Para remover os arquivos recebidos, log e objetos:

```bash
make clean
```