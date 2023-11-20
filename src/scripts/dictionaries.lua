-- INI
pollutant_dictionary = {}
ecological_dictionary = {}
pollutant_list_by_length = {}
ecological_list_by_length = {}

-- FUN
local function add_letter(dictionary, word, index)
    local table = dictionary
    for i = 1, index - 1 do
        table = table[string.sub(word, i, i)]
    end

    local char = string.sub(word, index, index)
    if (table[char] == nil) then
        table[char] = {}
    end

    if index == string.len(word) then 
        table[char].isValid = true
    end
end

local function build_dictionary_and_list(dictionary, list)
    -- reading lines from the stdin
    for word in io.lines() do
        local word_length = string.len(word)

        for i = 1, word_length do
            add_letter(dictionary, word, i)
        end

        if list[word_length] == nil then
            list[word_length] = {}
        end
        table.insert(list[word_length], word)
    end
end

local function verify_word(dictionary, word)
    local table = dictionary
    for i = 1, string.len(word) do
        if (table[string.sub(word, i, i)]) == nil then
            return false
        end

        table = table[string.sub(word, i, i)]
    end

    return table.isValid
end

function create_dictionaries()
    io.input("data/pollutant_words_list.txt")
    build_dictionary_and_list(pollutant_dictionary, pollutant_list_by_length)

    io.input("data/ecological_words_list.txt")
    build_dictionary_and_list(ecological_dictionary, ecological_list_by_length)
end

function is_word_pollutant(word)
    return verify_word(pollutant_dictionary, word)
end

function is_word_ecological(word)
    return verify_word(ecological_dictionary, word)
end

function swap_for_ecological_word(word_length)
    local words = nil
    local white_spaces_to_add = 0
    local word

    -- making sure we get a random word from an existing list.
    while words == nil do
        words = ecological_list_by_length[word_length]
        if words == nil then
            word_length = word_length - 1
            white_spaces_to_add = white_spaces_to_add + 1
        end
    end

    word = words[math.random(1, #words)]
    -- adding the trailing whitespaces necessary
    for i = 1, white_spaces_to_add, 1 do
        word = word .. ' '
    end

    return word
end

-- PGM
create_dictionaries()
