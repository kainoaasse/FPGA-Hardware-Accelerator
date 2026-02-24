----------------------------------------------------------------------------------
-- Company: George Mason
-- Engineer: Kainoa Asse
-- Create Date: 04/23/2024 10:22:07 AM
-- Revised:     02/24/2026 (Documentation and Final Verification)
-- Design Name: Sorting core on FPro System
-- Module Name: Sorting_datapath - Behavioral
-- Target Devices: Basys 3 (Artix-7)
-- Tool Versions: Vivado 2023.x / Vitis 2023.x
----------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity Sorting_datapath is
    Port (clk, Rd, WrInit : in std_logic;
          s : in std_logic; 
          DataIn : in std_logic_vector(15 downto 0);
          RAdd : in std_logic_vector(12 downto 0);
          --input N for loop counters
          N_in : in std_logic_vector(15 downto 0);
          --control signals from the controller
          Wr, Li, Ei, Lj, Ej : in std_logic;
          --added control signals for wrapper circuit 
          Done, addr_ctrl : in std_logic; --addr_ctrl signal for douta mux
          --control signals to the controller
          MigtMj, zi, zj : out std_logic;
          --datapath output          
          DataOut : out std_logic_vector(15 downto 0));
end Sorting_datapath;

architecture Behavioral of Sorting_datapath is
    signal wea : std_logic;
    signal AddrA : std_logic_vector(12 downto 0); --address going into address A of RAM
    signal icounter_out, jcounter_out : std_logic_vector(12 downto 0); --addresses coming from counter i and j
    signal jcounter_in : std_logic_vector(12 downto 0);
    signal dina : std_logic_vector(15 downto 0);
    signal Mi, Mj : std_logic_vector(15 downto 0);
    signal done_mux_out : std_logic_vector(15 downto 0); -- added signal for mux controlled by RdDone
    
begin

    RAM : entity work.RAM(Behavioral)
        Port Map(clk => clk,
                 wea => wea,
                 web => Wr,
                 addra => AddrA,
                 addrb => jcounter_out,
                 dina => dina,
                 dinb => Mi, --dinb always connected to Mi
                 douta => Mi,
                 doutb => Mj);
                 
    I_loop_counter : entity work.icounter(Behavioral)
        Port Map(clk => clk,
                 N => N_in(12 downto 0),
                 en => Ei,
                 ld => Li,
                 Q => icounter_out,
                 zi => zi);
                 
    jcounter_in <= std_logic_vector(unsigned(icounter_out) + 1); --input of i counter + 1 into j counter
    
    J_loop_counter : entity work.jcounter(Behavioral)
        Port Map(clk => clk,
                 N => N_in(12 downto 0),
                 en => Ej,
                 ld => Lj,
                 D => jcounter_in,
                 Q => jcounter_out,
                 zj => zj);

    Comparator_Block : entity work.Comparator(Behavioral)
        Port Map(A => Mi,
                 B => Mj,
                 AgtB => MigtMj);
    
    --multiplexing for address A of RAM
    AddrA <= icounter_out when (s = '1') else RAdd;
    
    --multiplexing for address dina of RAM
    dina <= Mj when (s = '1') else DataIn;
    
    --multiplexing for wea of RAM
    wea <= Wr when (s = '1') else WrInit;
    
    --multiplexing controlled by RdDone
    done_mux_out <= "000000000000000" & Done when (addr_ctrl = '1') else Mi;
    
    --multiplexing controlled by Rd
    DataOut <= done_mux_out when (Rd = '1') else (others => '0');
    
end Behavioral;
