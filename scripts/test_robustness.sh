#!/bin/bash

# Test de robustesse pour le serveur IRC
# Usage: ./test_robustness.sh
# Le serveur doit être lancé sur le port 4444 avec mdp "lol"

HOST="127.0.0.1"
PORT="4444"
PASS="lol"

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

passed=0
failed=0

test_cmd() {
    local name="$1"
    local cmds="$2"
    local expected="$3"
    
    echo -n "[$name] "
    # Ajoute QUIT à la fin pour fermer proprement et recevoir toutes les réponses
    result=$( { echo -e "$cmds"; sleep 0.3; echo -e "QUIT\r\n"; sleep 0.5; } | nc -w 3 $HOST $PORT 2>/dev/null)
    
    if echo "$result" | grep -q "$expected"; then
        echo -e "${GREEN}OK${NC}"
        ((passed++))
    else
        echo -e "${RED}FAIL${NC}"
        echo "  Expected: $expected"
        echo "  Got: $result"
        ((failed++))
    fi
    sleep 0.1
}

echo "========================================"
echo "  IRC Server Robustness Test"
echo "  Port: $PORT | Password: $PASS"
echo "========================================"
echo ""

# ==========================================
# 1. Tests d'authentification
# ==========================================
echo -e "${YELLOW}=== AUTHENTIFICATION ===${NC}"

test_cmd "PASS correct" \
    "PASS $PASS\r\nNICK testuser\r\nUSER test 0 * :Test\r\n" \
    "001"

test_cmd "PASS incorrect" \
    "PASS wrongpass\r\nNICK testuser2\r\nUSER test 0 * :Test\r\n" \
    "464"

test_cmd "Sans PASS" \
    "NICK nopass\r\nUSER test 0 * :Test\r\n" \
    "464"

test_cmd "PASS vide" \
    "PASS\r\nNICK nopass2\r\nUSER test 0 * :Test\r\n" \
    "464"

# ==========================================
# 2. Tests NICK
# ==========================================
echo -e "${YELLOW}=== NICK ===${NC}"

test_cmd "NICK vide" \
    "PASS $PASS\r\nNICK\r\nUSER test 0 * :Test\r\n" \
    "431"

test_cmd "NICK trop long (>9 chars)" \
    "PASS $PASS\r\nNICK verylongnn\r\nUSER test 0 * :Test\r\n" \
    "432"

test_cmd "NICK invalide (commence par chiffre)" \
    "PASS $PASS\r\nNICK 123nick\r\nUSER test 0 * :Test\r\n" \
    "432"

test_cmd "NICK avec @" \
    "PASS $PASS\r\nNICK nick@test\r\nUSER test 0 * :Test\r\n" \
    "432"

test_cmd "NICK avec #" \
    "PASS $PASS\r\nNICK nick#test\r\nUSER test 0 * :Test\r\n" \
    "432"

test_cmd "NICK avec espace" \
    "PASS $PASS\r\nNICK nick test\r\nUSER test 0 * :Test\r\n" \
    "432"

test_cmd "NICK valide" \
    "PASS $PASS\r\nNICK validnk\r\nUSER test 0 * :Test\r\n" \
    "001"

test_cmd "NICK avec underscore" \
    "PASS $PASS\r\nNICK nick_ok\r\nUSER test 0 * :Test\r\n" \
    "001"

test_cmd "NICK changement apres register" \
    "PASS $PASS\r\nNICK oldnick\r\nUSER test 0 * :Test\r\nNICK newnick\r\n" \
    "NICK :newnick"

# ==========================================
# 3. Tests USER
# ==========================================
echo -e "${YELLOW}=== USER ===${NC}"

test_cmd "USER pas assez de params" \
    "PASS $PASS\r\nNICK usertest\r\nUSER onlyone\r\n" \
    "461"

test_cmd "USER 2 params" \
    "PASS $PASS\r\nNICK usertes2\r\nUSER one two\r\n" \
    "461"

test_cmd "USER 3 params" \
    "PASS $PASS\r\nNICK usertes3\r\nUSER one two three\r\n" \
    "461"

test_cmd "USER deja registered" \
    "PASS $PASS\r\nNICK usertes4\r\nUSER test 0 * :Test\r\nUSER test2 0 * :Test2\r\n" \
    "462"

test_cmd "USER avec realname long" \
    "PASS $PASS\r\nNICK usertes5\r\nUSER test 0 * :This is a very long real name\r\n" \
    "001"

# ==========================================
# 4. Tests JOIN
# ==========================================
echo -e "${YELLOW}=== JOIN ===${NC}"

test_cmd "JOIN channel valide" \
    "PASS $PASS\r\nNICK joiner\r\nUSER test 0 * :Test\r\nJOIN #test\r\n" \
    "JOIN"

test_cmd "JOIN sans #" \
    "PASS $PASS\r\nNICK joiner2\r\nUSER test 0 * :Test\r\nJOIN nochannel\r\n" \
    "403"

test_cmd "JOIN sans argument" \
    "PASS $PASS\r\nNICK joiner3\r\nUSER test 0 * :Test\r\nJOIN\r\n" \
    "461"

test_cmd "JOIN channel avec &" \
    "PASS $PASS\r\nNICK joiner4\r\nUSER test 0 * :Test\r\nJOIN &local\r\n" \
    "JOIN"

test_cmd "JOIN multiple channels" \
    "PASS $PASS\r\nNICK joiner5\r\nUSER test 0 * :Test\r\nJOIN #chan1,#chan2\r\n" \
    "JOIN"

test_cmd "JOIN channel avec key" \
    "PASS $PASS\r\nNICK joiner6\r\nUSER test 0 * :Test\r\nJOIN #keychan secretkey\r\n" \
    "JOIN"

test_cmd "JOIN avant registration" \
    "PASS $PASS\r\nNICK joiner7\r\nJOIN #test\r\nUSER test 0 * :Test\r\n" \
    "001"

test_cmd "JOIN recoit NAMES list" \
    "PASS $PASS\r\nNICK joiner8\r\nUSER test 0 * :Test\r\nJOIN #namelist\r\n" \
    "353"

test_cmd "JOIN recoit END OF NAMES" \
    "PASS $PASS\r\nNICK joiner9\r\nUSER test 0 * :Test\r\nJOIN #endnames\r\n" \
    "366"

# ==========================================
# 5. Tests PRIVMSG
# ==========================================
echo -e "${YELLOW}=== PRIVMSG ===${NC}"

test_cmd "PRIVMSG sans destinataire" \
    "PASS $PASS\r\nNICK msgtest\r\nUSER test 0 * :Test\r\nPRIVMSG\r\n" \
    "461"

test_cmd "PRIVMSG user inexistant" \
    "PASS $PASS\r\nNICK msgtest2\r\nUSER test 0 * :Test\r\nPRIVMSG ghostuser :hello\r\n" \
    "401"

test_cmd "PRIVMSG channel inexistant" \
    "PASS $PASS\r\nNICK msgtest3\r\nUSER test 0 * :Test\r\nPRIVMSG #nochan :hello\r\n" \
    "403"

test_cmd "PRIVMSG sans texte" \
    "PASS $PASS\r\nNICK msgtest4\r\nUSER test 0 * :Test\r\nJOIN #msgchan\r\nPRIVMSG #msgchan\r\n" \
    "461"

test_cmd "PRIVMSG channel pas membre" \
    "PASS $PASS\r\nNICK msgtest5\r\nUSER test 0 * :Test\r\nPRIVMSG #otherchan :hello\r\n" \
    "403"

test_cmd "PRIVMSG avant registration" \
    "PASS $PASS\r\nNICK msgtest6\r\nPRIVMSG someone :hello\r\n" \
    "451"

# ==========================================
# 6. Tests MODE
# ==========================================
echo -e "${YELLOW}=== MODE ===${NC}"

test_cmd "MODE channel inexistant" \
    "PASS $PASS\r\nNICK modetest\r\nUSER test 0 * :Test\r\nMODE #nochan\r\n" \
    "403"

test_cmd "MODE +i sur channel" \
    "PASS $PASS\r\nNICK modeop\r\nUSER test 0 * :Test\r\nJOIN #modetest\r\nMODE #modetest +i\r\n" \
    "MODE"

test_cmd "MODE +t sur channel" \
    "PASS $PASS\r\nNICK modet\r\nUSER test 0 * :Test\r\nJOIN #modetopic\r\nMODE #modetopic +t\r\n" \
    "MODE"

test_cmd "MODE +k avec cle" \
    "PASS $PASS\r\nNICK modek\r\nUSER test 0 * :Test\r\nJOIN #modekey\r\nMODE #modekey +k secret\r\n" \
    "MODE"

test_cmd "MODE +l avec limite" \
    "PASS $PASS\r\nNICK model\r\nUSER test 0 * :Test\r\nJOIN #modelimit\r\nMODE #modelimit +l 10\r\n" \
    "MODE"

test_cmd "MODE +k sans cle" \
    "PASS $PASS\r\nNICK modek2\r\nUSER test 0 * :Test\r\nJOIN #modekey2\r\nMODE #modekey2 +k\r\n" \
    "461"

test_cmd "MODE +l sans limite" \
    "PASS $PASS\r\nNICK model2\r\nUSER test 0 * :Test\r\nJOIN #modelim2\r\nMODE #modelim2 +l\r\n" \
    "461"

test_cmd "MODE query (sans changement)" \
    "PASS $PASS\r\nNICK modeq\r\nUSER test 0 * :Test\r\nJOIN #modequery\r\nMODE #modequery\r\n" \
    "324"

test_cmd "MODE flag inconnu" \
    "PASS $PASS\r\nNICK modex\r\nUSER test 0 * :Test\r\nJOIN #modex\r\nMODE #modex +x\r\n" \
    "472"

test_cmd "MODE -i (retirer)" \
    "PASS $PASS\r\nNICK modem\r\nUSER test 0 * :Test\r\nJOIN #modeminus\r\nMODE #modeminus +i\r\nMODE #modeminus -i\r\n" \
    "MODE"

# ==========================================
# 7. Tests KICK/INVITE/TOPIC
# ==========================================
echo -e "${YELLOW}=== KICK ===${NC}"

test_cmd "KICK sans params" \
    "PASS $PASS\r\nNICK kicktest\r\nUSER test 0 * :Test\r\nKICK\r\n" \
    "461"

test_cmd "KICK channel inexistant" \
    "PASS $PASS\r\nNICK kicktes2\r\nUSER test 0 * :Test\r\nKICK #nochan someone\r\n" \
    "403"

test_cmd "KICK user inexistant" \
    "PASS $PASS\r\nNICK kicktes3\r\nUSER test 0 * :Test\r\nJOIN #kickchan\r\nKICK #kickchan nobody\r\n" \
    "441"

echo -e "${YELLOW}=== INVITE ===${NC}"

test_cmd "INVITE sans params" \
    "PASS $PASS\r\nNICK invtest\r\nUSER test 0 * :Test\r\nINVITE\r\n" \
    "461"

test_cmd "INVITE 1 param" \
    "PASS $PASS\r\nNICK invtest2\r\nUSER test 0 * :Test\r\nINVITE someone\r\n" \
    "461"

test_cmd "INVITE user inexistant" \
    "PASS $PASS\r\nNICK invtest3\r\nUSER test 0 * :Test\r\nJOIN #invchan\r\nINVITE ghost #invchan\r\n" \
    "401"

test_cmd "INVITE channel inexistant" \
    "PASS $PASS\r\nNICK invtest4\r\nUSER test 0 * :Test\r\nINVITE someone #nochan\r\n" \
    "401"

echo -e "${YELLOW}=== TOPIC ===${NC}"

test_cmd "TOPIC channel inexistant" \
    "PASS $PASS\r\nNICK topictes\r\nUSER test 0 * :Test\r\nTOPIC #nochan\r\n" \
    "403"

test_cmd "TOPIC sans argument" \
    "PASS $PASS\r\nNICK topicte2\r\nUSER test 0 * :Test\r\nTOPIC\r\n" \
    "461"

test_cmd "TOPIC query (pas de topic)" \
    "PASS $PASS\r\nNICK topicte3\r\nUSER test 0 * :Test\r\nJOIN #notopic\r\nTOPIC #notopic\r\n" \
    "331"

test_cmd "TOPIC set" \
    "PASS $PASS\r\nNICK topicte4\r\nUSER test 0 * :Test\r\nJOIN #settopic\r\nTOPIC #settopic :New Topic\r\n" \
    "TOPIC"

test_cmd "TOPIC query apres set" \
    "PASS $PASS\r\nNICK topicte5\r\nUSER test 0 * :Test\r\nJOIN #topicq\r\nTOPIC #topicq :My Topic\r\nTOPIC #topicq\r\n" \
    "332"

test_cmd "TOPIC pas dans channel" \
    "PASS $PASS\r\nNICK topicte6\r\nUSER test 0 * :Test\r\nJOIN #topicin\r\nPART #topicin\r\nTOPIC #topicin\r\n" \
    "403"

# ==========================================
# 8. Tests PART/QUIT
# ==========================================
echo -e "${YELLOW}=== PART ===${NC}"

test_cmd "PART sans argument" \
    "PASS $PASS\r\nNICK parttest\r\nUSER test 0 * :Test\r\nPART\r\n" \
    "461"

test_cmd "PART channel pas dedans" \
    "PASS $PASS\r\nNICK parttes2\r\nUSER test 0 * :Test\r\nPART #notjoined\r\n" \
    "403"

test_cmd "PART channel valide" \
    "PASS $PASS\r\nNICK parttes3\r\nUSER test 0 * :Test\r\nJOIN #partchan\r\nPART #partchan\r\n" \
    "PART"

test_cmd "PART avec raison" \
    "PASS $PASS\r\nNICK parttes4\r\nUSER test 0 * :Test\r\nJOIN #partreason\r\nPART #partreason :Goodbye!\r\n" \
    "PART"

test_cmd "PART multiple channels" \
    "PASS $PASS\r\nNICK parttes5\r\nUSER test 0 * :Test\r\nJOIN #part1,#part2\r\nPART #part1,#part2\r\n" \
    "PART"

echo -e "${YELLOW}=== QUIT ===${NC}"

test_cmd "QUIT simple" \
    "PASS $PASS\r\nNICK quittest\r\nUSER test 0 * :Test\r\n" \
    "001"

test_cmd "QUIT avec message" \
    "PASS $PASS\r\nNICK quitte2\r\nUSER test 0 * :Test\r\n" \
    "001"

# ==========================================
# 9. Tests PING/PONG
# ==========================================
echo -e "${YELLOW}=== PING/PONG ===${NC}"

test_cmd "PING simple" \
    "PASS $PASS\r\nNICK pingtest\r\nUSER test 0 * :Test\r\nPING :test123\r\n" \
    "PONG"

test_cmd "PING avec token" \
    "PASS $PASS\r\nNICK pingtes2\r\nUSER test 0 * :Test\r\nPING mytoken\r\n" \
    "mytoken"

test_cmd "PING sans token" \
    "PASS $PASS\r\nNICK pingtes3\r\nUSER test 0 * :Test\r\nPING\r\n" \
    "PONG"

# ==========================================
# 10. Tests CAP (capability negotiation)
# ==========================================
echo -e "${YELLOW}=== CAP ===${NC}"

test_cmd "CAP LS" \
    "CAP LS\r\nPASS $PASS\r\nNICK captest\r\nUSER test 0 * :Test\r\nCAP END\r\n" \
    "001"

test_cmd "CAP REQ (refusé)" \
    "CAP LS\r\nCAP REQ :multi-prefix\r\nCAP END\r\nPASS $PASS\r\nNICK captes2\r\nUSER test 0 * :Test\r\n" \
    "NAK"

# ==========================================
# 11. Tests commandes non enregistré
# ==========================================
echo -e "${YELLOW}=== COMMANDES SANS REGISTRATION ===${NC}"

test_cmd "JOIN avant USER" \
    "PASS $PASS\r\nNICK unreg1\r\nJOIN #test\r\n" \
    ""

test_cmd "PRIVMSG avant USER" \
    "PASS $PASS\r\nNICK unreg2\r\nPRIVMSG someone :hello\r\n" \
    "451"

test_cmd "MODE avant USER" \
    "PASS $PASS\r\nNICK unreg3\r\nMODE #test\r\n" \
    ""

# ==========================================
# 12. Tests EDGE CASES / STRESS
# ==========================================
echo -e "${YELLOW}=== EDGE CASES ===${NC}"

# Commande inconnue
test_cmd "Commande inconnue" \
    "PASS $PASS\r\nNICK unknown\r\nUSER test 0 * :Test\r\nFAKECOMMAND arg\r\n" \
    "001"

# Ligne vide - le serveur doit ignorer les lignes vides et continuer
test_cmd "Lignes vides" \
    "PASS $PASS\r\nNICK emptyln\r\nUSER test 0 * :Test\r\n\r\n\r\n" \
    "001"

# Channel avec caracteres speciaux valides
test_cmd "Channel #test-ok" \
    "PASS $PASS\r\nNICK chansp1\r\nUSER test 0 * :Test\r\nJOIN #test-ok\r\n" \
    "JOIN"

test_cmd "Channel #test_ok" \
    "PASS $PASS\r\nNICK chansp2\r\nUSER test 0 * :Test\r\nJOIN #test_ok\r\n" \
    "JOIN"

# Double JOIN meme channel
test_cmd "Double JOIN meme channel" \
    "PASS $PASS\r\nNICK dbljoin\r\nUSER test 0 * :Test\r\nJOIN #double\r\nJOIN #double\r\n" \
    "443"

# Commande en minuscule
test_cmd "Commande minuscule (join)" \
    "PASS $PASS\r\nNICK lowerc\r\nUSER test 0 * :Test\r\njoin #lower\r\n" \
    ""

# Message avec caracteres speciaux
test_cmd "Message avec accents" \
    "PASS $PASS\r\nNICK accent\r\nUSER test 0 * :Test\r\nJOIN #accents\r\nPRIVMSG #accents :Café résumé\r\n" \
    "001"

# ==========================================
# 13. Test connexions rapides
# ==========================================
echo -e "${YELLOW}=== CONNEXIONS RAPIDES ===${NC}"
echo -n "[10 connexions rapides] "
for i in {1..10}; do
    echo -e "PASS $PASS\r\nNICK rapid$i\r\nUSER test 0 * :Test\r\nQUIT\r\n" | nc -w 1 -q 1 $HOST $PORT &>/dev/null &
done
wait
echo -e "${GREEN}OK${NC}"
((passed++))

# ==========================================
# Résumé
# ==========================================
echo ""
echo "========================================"
echo -e "  Résultats: ${GREEN}$passed passés${NC} / ${RED}$failed échoués${NC}"
echo "========================================"

exit $failed
