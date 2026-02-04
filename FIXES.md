# Résumé des corrections apportées

## 1. Bug critique dans `Parser::getCommand()` (Parser.cpp)

### ❌ Code original (cassé)
```cpp
Command Parser::getCommand(std::string cmdline)
{
    Command cmd;
    try
    {
        std::size_t index = cmdline.find(":");  // ⚠️ Cherche ':' N'IMPORTE OÙ
        std::size_t end;
        if (index == std::string::npos)
        {
            cmd.setPrefix("");
            index = 0;
            end = 0;
        }
        else
        {
            end = cmdline.find(" ");
            cmd.setPrefix(cmdline.substr(index, end));  // ⚠️ substr(pos, len) mal utilisé
            cmdline = cmdline.substr(end);              // ⚠️ Ne skip pas l'espace
        }
        index = end;
        end = cmdline.find(" ");
        if (end == std::string::npos)
        {
            cmd.setCmd("");
            throw std::runtime_error("No command found");
        }
        else
        {
            cmd.setCmd(cmdline.substr(index, end));  // ⚠️ index et end utilisés comme positions
            cmdline = cmdline.substr(end);
        }
        // ... suite cassée aussi
    }
}
```

### Pourquoi c'était cassé ?

Pour la commande `USER test 0 * :Test`:

1. `cmdline.find(":")` trouve `:` à la position 14 (dans `:Test`)
2. Le code pense qu'il y a un **préfixe IRC** (qui commence par `:`)
3. Il entre dans le `else` et fait n'importe quoi avec les indices
4. Résultat: `cmd.getCmd()` retourne une chaîne **vide**
5. `dispatch()` ne reconnaît pas la commande USER
6. Le client n'est jamais enregistré → pas de messages 001-004
7. Irssi dit "Not connected to server"

### ✅ Code corrigé
```cpp
Command Parser::getCommand(std::string cmdline)
{
    Command cmd;
    try
    {
        std::size_t index = 0;
        std::size_t end;
        
        // ✅ Vérifie si ':' est AU DÉBUT de la ligne (vrai préfixe IRC)
        if (!cmdline.empty() && cmdline[0] == ':')
        {
            end = cmdline.find(" ");
            if (end == std::string::npos) {
                cmd.setCmd("");
                throw std::runtime_error("No command found after prefix");
            }
            cmd.setPrefix(cmdline.substr(0, end));
            cmdline = cmdline.substr(end + 1);  // ✅ Skip l'espace
        }
        else
        {
            cmd.setPrefix("");
        }
        
        // ✅ Extraction correcte de la commande
        end = cmdline.find(" ");
        if (end == std::string::npos)
        {
            cmd.setCmd(cmdline);  // ✅ Commande sans arguments (ex: QUIT)
            cmd.setArgs("");
            cmd.setTrailing("");
            return (cmd);
        }
        else
        {
            cmd.setCmd(cmdline.substr(0, end));  // ✅ substr(0, longueur)
            cmdline = cmdline.substr(end + 1);   // ✅ Skip l'espace
        }
        
        // ✅ Extraction args et trailing correcte
        end = cmdline.find(":");
        if (end == std::string::npos)
        {
            cmd.setArgs(cmdline); 
            cmd.setTrailing("");
            return (cmd);
        }
        else
        {
            if (end > 0)
                cmd.setArgs(cmdline.substr(0, end - 1));
            else
                cmd.setArgs("");
            cmd.setTrailing(cmdline.substr(end + 1));
        }
    }
}
```

---

## 2. Bug dans `Dispatch::ft_pass()` (Dispatch.cpp)

### ❌ Code cassé
```cpp
if (client->isRegistered()) {
    std::string txt = ":server 462 * :You may not reregister\r\n";
    send(fd, txt.c_str(), txt.length(), 0);
    return fals    // ⚠️ ERREUR DE SYNTAXE: "fals" au lieu de "false;"
std::string line = cmd.getLine();  // ⚠️ Pas de } ni de ;
```

### ✅ Code corrigé
```cpp
if (client->isRegistered()) {
    std::string txt = ":server 462 * :You may not reregister\r\n";
    send(fd, txt.c_str(), txt.length(), 0);
    return false;  // ✅ Syntaxe correcte
}

std::string line = cmd.getLine();
```

---

## 3. Messages IRC mal formatés (Dispatch.cpp)

### ❌ Avant
```cpp
std::string msg = "431 :No nickname given\r\n";           // Pas de préfixe serveur
std::string msg = "serveur 433 * " + nick + " :...";     // Typo "serveur"
std::string msg = "461 :Not enough parameters\r\n";       // Pas de préfixe
```

### ✅ Après
```cpp
std::string msg = ":server 431 * :No nickname given\r\n";
std::string msg = ":server 433 * " + nick + " :Nickname is already in use\r\n";
std::string msg = ":server 461 USER :Not enough parameters\r\n";
```

---

## Leçon à retenir

Le format IRC d'un message avec préfixe:
```
:prefix COMMAND args :trailing
```

Le préfixe commence **toujours** par `:` **au début de la ligne**.

Le `:` dans le trailing (ex: `USER test 0 * :Real Name`) n'est **PAS** un préfixe!

Le code original confondait ces deux cas car il cherchait `:` n'importe où avec `find(":")`.
