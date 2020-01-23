def delimited_reader(reader, delimiter: str):
    """This generator reads a file word by word"""
    if not len(delimiter) == 1:
        raise AssertionError('Delimiter is not a char!')
    while True:
        word = ''
        letter = reader.read(1)
        while letter not in (delimiter, ''):
            word += letter
            letter = reader.read(1)
        if word == letter == '':
            return
        else:
            yield word


class Mat:
    """Represents a lattice struct"""

    def __init__(self, size=0):
        self.size = size
        self.mat = [0] * size * size

    def load(self, file: str):
        """Loads lattice values from given file"""
        reader = delimited_reader(open(file), ' ')
        self.size = int(reader.__next__())
        self.mat = []
        for word in reader:
            self.mat.append(float(word))
            if len(self.mat) == self.size ** 2:
                break
        for i in range(self.size):
            for j in range(i):
                self[i, j] = self[j, i]

    def hamiltonian(self, spins: list):
        """Calculates hamiltonian of given spin configuration"""
        if len(spins) != self.size:
            raise ValueError('Spin count ({}) does not correspond to mat size ({})'.format(len(spins), self.size))
        hamiltonian = 0.
        for i in range(self.size):
            hamiltonian += self[i, i] * spins[i]
            for j in range(i + 1, self.size):
                hamiltonian += self[i, j] * spins[i] * spins[j]
        return hamiltonian

    def __getitem__(self, item):
        """A nice way to get a lattice element by index"""
        return self.mat[item[0] * self.size + item[1]]

    def __setitem__(self, idx, value):
        """A nice way to set a lattice value"""
        self.mat[idx[0] * self.size + idx[1]] = value


# Load lattice
mat_filename = input('Enter mat filename >> ')
mat = Mat()
mat.load(mat_filename)

while True:
    spin_set = [1 if float(word) > 0 else -1 for word in input('Enter spin data >> ').split()]
    local_minimum = True
    base_hamiltonian = mat.hamiltonian(spin_set)
    for spin_idx in range(mat.size):
        spin_set[spin_idx] *= -1
        new_hamiltonian = mat.hamiltonian(spin_set)
        spin_set[spin_idx] *= -1
        if base_hamiltonian > new_hamiltonian:
            local_minimum = False
            print('Hamiltonian decrease from {} to {} detected when toggling spin #{} value'.
                  format(base_hamiltonian, new_hamiltonian, spin_idx))
    print({
              True: 'Spin configuration is a local minimum; Hamiltonian: {}'.format(base_hamiltonian),
              False: 'Spin configuration is not a local minimum'
          }[local_minimum])
