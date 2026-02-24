----------------------------------------------------------------------------------
-- Company: George Mason
-- Engineer: Kainoa Asse
-- Create Date: 04/22/2024 
-- Revised:     02/24/2026 (Documentation and Final Verification)
-- Design Name: Sorting core on FPro System
-- Module Name: icounter - Behavioral
-- Target Devices: Basys 3 (Artix-7)
-- Tool Versions: Vivado 2023.x / Vitis 2023.x
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity icounter is
    Port(clk, en, ld : in std_logic;
         N : in std_logic_vector(12 downto 0);
         Q : out std_logic_vector(12 downto 0);
         zi : out std_logic
         );
end icounter;

architecture Behavioral of icounter is
    
    signal count : unsigned(12 downto 0) := (OTHERS => '0');
    
begin
    process(clk) 
    begin
        if(rising_edge(clk)) then
            if(en = '1') then
			    if(ld = '1') then
				    count <= (OTHERS => '0'); --Load 0 to counter
			    else
				    count <= count + 1;
			    end if;	
            end if;
        end if;
    end process;

    Q <= std_logic_vector(count);
    zi <= '1' when count = (unsigned(N) - 2) else '0'; -- zi = '1' when i loop finished 
end Behavioral;